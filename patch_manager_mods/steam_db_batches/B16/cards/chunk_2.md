# Review cards B16 chunk 2

Answer each card with one TSV row in `steam_db_batches/B16/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B16-2-01

GOG function: CSWGuiInGameGameplay::OnExitDiff  gog 0x005e6080  callconv=(blank)  param_size_bytes=0  gog_size=70  gog_retn=4
Candidate Steam address: 0x008e1810 (size 1702, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.14) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.03 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x008e1ec0:STR0/CALLEE0/SIG0.083/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x005e6020 CSWGuiInGameGameplay::OnEnterDiff -> 0x008e1570  gate_class=REVIEW
- GOG-order neighbour after:  0x005e60d0 CSWGuiInGameGameplay::SetUpOptions -> 0x008e1620  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x009153e0 CSWGuiTextParams::SetColor -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x38} | Steam {0x48,0x68,0x98,0xc4,0x38c,0x654,0x91c,0xbe4,0xeac,0x1118,0x1f39,0x1f44}
imm sets (* = in both, 3 common): GOG {0x4*,0x6*,0x8*,0xea0,0xffffffbf} | Steam {0x2,0x3,0x4*,0x5,0x6*,0x7,0x8*,0x9,0xa,0xb,0xc,0xd,0xe,0xf,...(+34)}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 4)
```

### prologue
```
prologue for 0x008e1810: 16-byte aligned: YES
  008e1810  55             PUSH EBP
  008e1811  8B EC          MOV EBP,ESP
  008e1813  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x008e1810
string literals (14):
  0x009A8FB8  at 008e1913  'optautopause_p'
  0x009A8FAC  at 008e1933  'CB_ENDROUND'
  0x009A8F9C  at 008e196d  'CB_ENEMYSIGHTED'
  0x009A8F8C  at 008e19aa  'CB_MINESIGHTED'
  0x009A8F7C  at 008e19e7  'CB_PARTYKILLED'
  0x009A8F6C  at 008e1a23  'CB_ACTIONMENU'
  0x009A8F60  at 008e1a60  'CB_TRIGGERS'
  0x009A1818  at 008e1a9d  'LBL_TITLE'
  0x009A8F54  at 008e1ad9  'LB_DETAILS'
  0x009A18AC  at 008e1b16  'BTN_DEFAULT'
  0x0099E548  at 008e1b8f  'LBL_BAR1'
  0x0099E53C  at 008e1bcc  'LBL_BAR2'
direct callees (16):
  0x0040E5A0  CSWGuiPanel::Constructor
  0x0091D67E  (unnamed)
  0x00419740  (unnamed)
  0x0041A510  (unnamed)
  0x0041CE90  (unnamed)
  0x00710840  (unnamed)
  0x0040EE40  CSWGuiPanel::StartLoadFromLayout
  0x00733570  CExoString::CExoString_char
  0x0040F620  CSWGuiPanel::InitControl
  0x00733780  CExoString::~CExoString
  0x00418AF0  CSWGuiControl::AddEvent
  0x0040F5A0  CSWGuiPanel::StopLoadFromLayout
  0x007D2360  (unnamed)
  0x00760EA0  (unnamed)
  0x008E2060  (unnamed)
  0x0040EC00  CSWGuiPanel::SetActiveControl
struct offsets touched (top 10):
  +0x48 [dec 72]  x4  via EAX,ECX,EDX
  +0x98 [dec 152]  x1  via EAX
  +0xC4 [dec 196]  x1  via EDX
  +0x38C [dec 908]  x1  via EAX
  +0x654 [dec 1620]  x1  via ECX
  +0x91C [dec 2332]  x1  via EDX
  +0xBE4 [dec 3044]  x1  via EAX
  +0xEAC [dec 3756]  x1  via ECX
  +0x1118 [dec 4376]  x1  via EDX
  +0x1F39 [dec 7993]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 008e1810 FUN_008e1810 size=1702 ret=4 callers=1 callees=16

undefined4 * __thiscall FUN_008e1810(undefined4 *param_1,undefined4 param_2)

{
  uint uVar1;
  undefined1 local_98 [8];
  undefined1 local_90 [8];
  undefined1 local_88 [8];
  undefined1 local_80 [8];
  undefined1 local_78 [8];
  undefined1 local_70 [8];
  undefined1 local_68 [8];
  undefined1 local_60 [8];
  undefined1 local_58 [8];
  undefined1 local_50 [8];
  undefined1 local_48 [8];
  undefined1 local_40 [8];
  undefined1 local_38 [8];
  undefined1 local_30 [8];
  undefined1 local_28 [16];
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0097db93;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_0040e5a0(param_2);
  local_8 = 0;
  *param_1 = CSWGuiInGameAutoPause::vftable;
  _eh_vector_constructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable](param_1 + 0x1a,0x2c8,6,FUN_008e1ef0,FUN_007d5230);
  local_8._0_1_ = 1;
  FUN_00419740(uVar1);
  local_8._0_1_ = 2;
  FUN_00419740();
  local_8._0_1_ = 3;
  FUN_0041a510();
  local_8._0_1_ = 4;
  FUN_0041a510();
  local_8._0_1_ = 5;
  _eh_vector_constructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable]
            (param_1 + 0x5d2,0x148,4,FUN_00419740,
             CMFCToolBarButtonsListButton::~CMFCToolBarButtonsListButton);
  local_8._0_1_ = 6;
  FUN_0041ce90();
  local_8._0_1_ = 7;
  FUN_00710840("optautopause_p");
  FUN_0040ee40(local_28,1,1);
  FUN_00733570("CB_ENDROUND");
  local_8._0_1_ = 8;
  FUN_0040f620(param_1 + 0x1a,local_30,1,1);
  local_8._0_1_ = 7;
  FUN_00733780();
  FUN_00733570("CB_ENEMYSIGHTED");
  local_8._0_1_ = 9;
  FUN_0040f620(param_1 + 0xcc,local_38,1,1);
  local_8._0_1_ = 7;
  FUN_00733780();
  FUN_00733570("CB_MINESIGHTED");
  local_8._0_1_ = 10;
  FUN_0040f620(param_1 + 0x17e,local_40,1,1);
  local_8._0_1_ = 7;
  FUN_00733780();
  FUN_00733570("CB_PARTYKILLED");
  local_8._0_1_ = 0xb;
  FUN_0040f620(param_1 + 0x230,local_48,1,1);
  local_8._0_1_ = 7;
  FUN_00733780();
  FUN_00733570("CB_ACTIONMENU");
  local_8._0_1_ = 0xc;
  FUN_0040f620(param_1 + 0x2e2,local_50,1,1);
  local_8._0_1_ = 7;
  FUN_00733780();
  FUN_00733570("CB_TRIGGERS");
  local_8._0_1_ = 0xd;
  FUN_0040f620(param_1 + 0x394,local_58,1,1);
/* ... 67 more lines truncated by --max-lines */
```

### decoy: 0x008e1ec0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008e1ec0: 16-byte aligned: YES
  008e1ec0  55             PUSH EBP
  008e1ec1  8B EC          MOV EBP,ESP
  008e1ec3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008e1ec0
string literals (0):
direct callees (2):
  0x008E1F10  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card B16-2-02

GOG function: CAurTextureBasic::ParseField  gog 0x008161b0  callconv=(blank)  param_size_bytes=4  gog_size=3688  gog_retn=4
Candidate Steam address: 0x00423ab0 (size 3688, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CAurTextureBasic::ParseTXILine | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 33 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,SIG  (content level C2)
- evidence summary: STR 'specularbumpintensity'@00988154; 'diffusebumpintensity'@0098816c; 'envmapalpha'@00988184; SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00424920:STR0/CALLEE0/SIG0.08/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00816090 CAurTextureBasic::Reset -> 0x00423990  gate_class=AUTO
- GOG-order neighbour after:  0x00817020 LifeTextureController::Destructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (19), mapped to Steam through trusted rows only:
  0x00817660 CAurFontInfo::ParseField -> Steam ?  in candidate callees: n/a
  0x008c5260 Global::AurMessageGetToken -> Steam ?  in candidate callees: n/a
  0x008c52e0 Global::AurMessageParseVector -> Steam ?  in candidate callees: n/a
  0x008c5310 Global::AurMessageParseString -> Steam ?  in candidate callees: n/a
  0x008c5350 Global::AurMessageParseFloat -> Steam ?  in candidate callees: n/a
  0x008c5370 Global::AurMessageParseIntOrBool -> Steam ?  in candidate callees: n/a
  0x008c5430 Global::Parse_3 -> Steam ?  in candidate callees: n/a
  0x008c5460 Global::Parse_4 -> Steam ?  in candidate callees: n/a
  ... 11 more GOG callees
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (33): 'gamma'; 'specularbumpintensity'; 'diffusebumpintensity'; 'envmapalpha' ...
disp sets (* = in both, 9 common): GOG {0xc*,0x34*,0x38*,0x3c*,0x40*,0x44*,0xbe*,0xc0*,0xf0*} | Steam {0xc*,0x34*,0x38*,0x3c*,0x40*,0x44*,0xbe*,0xc0*,0xf0*}
imm sets (* = in both, 41 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0x18*,0x1c*,0x20*,0x24*,0x28*,...(+27)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0x18*,0x1c*,0x20*,0x24*,0x28*,...(+27)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00423ab0: 16-byte aligned: YES
  00423ab0  55             PUSH EBP
  00423ab1  8B EC          MOV EBP,ESP
  00423ab3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00423ab0
string literals (57):
  0x009883D0  at 00423af2  'proceduretype'
  0x009883D0  at 00423b07  'proceduretype'
  0x009883C8  at 00423b85  'dirty'
  0x0098839C  at 00423c31  'dirty2'
  0x00988394  at 00423cdd  'dirty3'
  0x0098838C  at 00423d89  'water'
  0x0098835C  at 00423e7c  'perlin'
  0x00988330  at 00423f09  'arturo'
  0x00988320  at 00423fff  'cycle'
  0x00988318  at 0042407a  'random'
  0x009882E8  at 00424125  'ringtexdistort'
  0x009882BC  at 004241db  'filerange'
direct callees (18):
  0x00484D10  Global::AurMessageGetToken
  0x009370C7  _stricmp
  0x00919F10  _strlen
  0x00484DC0  (unnamed)
  0x0091D780  (unnamed)
  0x00919723  operator new
  0x0048F710  TextureController::ctor
  0x00490B80  (unnamed)
  0x004927D0  (unnamed)
  0x00491B50  (unnamed)
  0x00493370  (unnamed)
  0x00484EE0  (unnamed)
  0x00484F10  (unnamed)
  0x00484E00  (unnamed)
  0x00484E20  (unnamed)
  0x00484D90  (unnamed)
  0x00424F60  CAurFontInfo::ParseTXIKey
  0x009196EE  __security_check_cookie
struct offsets touched (top 9):
  +0x34 [dec 52]  x17  via EAX,ECX,EDX
  +0x38 [dec 56]  x6  via EAX,EDX
  +0x3C [dec 60]  x5  via ECX
  +0x40 [dec 64]  x3  via EDX
  +0x44 [dec 68]  x3  via EAX
  +0xF0 [dec 240]  x3  via ECX
  +0xBE [dec 190]  x2  via ECX
  +0xC [dec 12]  x1  via EDX
  +0xC0 [dec 192]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00423ab0 FUN_00423ab0 size=3688 ret=4 callers=2 callees=19

void __thiscall FUN_00423ab0(int param_1,int param_2)

{
  char *pcVar1;
  int iVar2;
  size_t sVar3;
  undefined4 *local_ec;
  undefined4 *local_e8;
  undefined4 local_e4;
  undefined4 local_e0;
  undefined4 local_dc;
  undefined4 *local_d8;
  undefined4 *local_d4;
  undefined4 local_d0;
  undefined4 *local_cc;
  undefined4 *local_c8;
  undefined4 *local_c4;
  char local_58 [68];
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094901e;
  local_10 = ExceptionList;
  local_14 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_58[0] = '\0';
  pcVar1 = (char *)FUN_00484d10(param_2,local_14);
  iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("proceduretype",pcVar1);
  if (iVar2 == 0) {
    pcVar1 = local_58;
    sVar3 = _strlen [GHIDRA-AUTO-LABEL: unreliable]("proceduretype");
    FUN_00484dc0(sVar3 + param_2,pcVar1);
  }
  sVar3 = _strlen [GHIDRA-AUTO-LABEL: unreliable](local_58);
  if (sVar3 != 0) {
    if (*(int *)(param_1 + 0x34) != 0) {
      if (*(undefined4 **)(param_1 + 0x34) != (undefined4 *)0x0) {
        (**(code **)**(undefined4 **)(param_1 + 0x34))(1);
      }
      *(undefined4 *)(param_1 + 0x34) = 0;
    }
    iVar2 = _strcmp [GHIDRA-AUTO-LABEL: unreliable](local_58,"dirty");
    if (iVar2 == 0) {
      local_c4 = (undefined4 *)FUN_00919723(0x48);
      local_8 = 0;
      if (local_c4 == (undefined4 *)0x0) {
        local_c4 = (undefined4 *)0x0;
      }
      else {
        FUN_0048f710(param_1);
        *local_c4 = DirtyTextureController::vftable;
        local_c4[0xe = byte +0x38] = 0;
        local_c4[0x11 = byte +0x44] = 0;
        local_c4[0xf = byte +0x3c] = 0;
        *(undefined1 *)(local_c4 + 0x10) = 0;
      }
      local_8 = 0xffffffff;
      *(undefined4 **)(param_1 + 0x34) = local_c4;
      *(undefined1 *)(param_1 + 0xf0) = 1;
    }
    iVar2 = _strcmp [GHIDRA-AUTO-LABEL: unreliable](local_58,"dirty2");
    if (iVar2 == 0) {
      local_c8 = (undefined4 *)FUN_00919723(0x48);
      local_8 = 1;
      if (local_c8 == (undefined4 *)0x0) {
        local_c8 = (undefined4 *)0x0;
      }
      else {
        FUN_0048f710(param_1);
        *local_c8 = DirtyTextureController::vftable;
        local_c8[0xe = byte +0x38] = 0;
        local_c8[0x11 = byte +0x44] = 0;
        local_c8[0xf = byte +0x3c] = 1;
        *(undefined1 *)(local_c8 + 0x10) = 0;
      }
      local_8 = 0xffffffff;
/* ... 328 more lines truncated by --max-lines */
```

### decoy: 0x00424920 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00424920: 16-byte aligned: YES
  00424920  55             PUSH EBP
  00424921  8B EC          MOV EBP,ESP
  00424923  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00424920
string literals (0):
direct callees (2):
  0x009196FD  operator delete
  0x0048F7F0  (unnamed)
struct offsets touched (top 1):
  +0x38 [dec 56]  x1  via ECX
```

## Card B16-2-03

GOG function: CExoMoviePlayerInternal::StopMovie  gog 0x00920c10  callconv=(blank)  param_size_bytes=0  gog_size=29  gog_retn=0
Candidate Steam address: 0x0040dac0 (size 390, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0040da20:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x009208e0 CExoMoviePlayerInternal::RunMovieFrameLoop -> 0x0040d6f0  gate_class=REVIEW
- GOG-order neighbour after:  0x00920c30 CExoMoviePlayerInternal::PlayMoviesAsync -> 0x0040da40  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0091fd10 CExoMoviePlayerInternal::ShutDown -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x30} | Steam {0x4,0x8,0x44,0x50}
imm sets (* = in both, 0 common): GOG {} | Steam {0x3,0x4,0x5,0xc,0x38,0x40,0x41,0x54,0x1c4}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0040dac0: 16-byte aligned: YES
  0040dac0  55             PUSH EBP
  0040dac1  8B EC          MOV EBP,ESP
  0040dac3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0040dac0
string literals (0):
direct callees (7):
  0x00739CF0  (unnamed)
  0x00919723  operator new
  0x0073FEA0  CClientExoApp::GetSWGuiManager
  0x007BC600  (unnamed)
  0x00410530  CSWGuiManager::AddPanel
  0x00409ED0  (unnamed)
  0x00739D10  (unnamed)
struct offsets touched (top 4):
  +0x44 [dec 68]  x5  via EAX,ECX,EDX
  +0x4 [dec 4]  x3  via ECX,EDX
  +0x8 [dec 8]  x1  via EDX
  +0x50 [dec 80]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0040dac0 FUN_0040dac0 size=390 ret=0 callers=1 callees=7

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0040dac0(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 local_48;
  undefined4 local_3c;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094823b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00739cf0(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  if (*(int *)(param_1 + 0x44) == 0) {
    iVar1 = FUN_00919723(0x1c4);
    local_8 = 0;
    if (iVar1 == 0) {
      local_3c = 0;
    }
    else {
      uVar2 = FUN_0073fea0();
      local_3c = FUN_007bc600(uVar2);
    }
    local_8 = 0xffffffff;
    *(undefined4 *)(param_1 + 0x44) = local_3c;
    iVar1 = *(int *)(param_1 + 0x44);
    *(undefined4 *)(iVar1 + 0x54) = 0;
    *(undefined4 *)(iVar1 + 0x58) = 0;
    *(undefined4 *)(iVar1 + 0x5c) = 0;
  }
  uVar4 = 1;
  uVar3 = 3;
  uVar2 = *(undefined4 *)(param_1 + 0x44);
  FUN_0073fea0(uVar2,3,1);
  FUN_00410530(uVar2,uVar3,uVar4);
  if (1.0 <= (float)_DAT_00986b38) {
    local_48 = 0;
  }
  else {
    local_48 = 0x3f800000;
  }
  *(undefined4 *)(*(int *)(param_1 + 0x44) + 0x50) = local_48;
  FUN_00409ed0(_DAT_00986da8,0,1);
  FUN_00739d10();
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x0040da20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0040da20: 16-byte aligned: YES
  0040da20  55             PUSH EBP
  0040da21  8B EC          MOV EBP,ESP
  0040da23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0040da20
string literals (0):
direct callees (1):
  0x0040CB20  (unnamed)
struct offsets touched (top 1):
  +0x30 [dec 48]  x1  via EAX
```

## Card B16-2-04

GOG function: CExoMoviePlayerInternal::RestartMoviesAsync  gog 0x00921080  callconv=(blank)  param_size_bytes=0  gog_size=64  gog_retn=0
Candidate Steam address: 0x0040deb0 (size 64, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x0040def0:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00921040 CExoMoviePlayerInternal::ResumeMovie -> 0x0040de50  gate_class=REVIEW
- GOG-order neighbour after:  0x009210c0 CExoMoviePlayer::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x30*,0x38*} | Steam {0x4*,0x30*,0x38*}
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
prologue for 0x0040deb0: 16-byte aligned: YES
  0040deb0  55             PUSH EBP
  0040deb1  8B EC          MOV EBP,ESP
  0040deb3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0040deb0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x38 [dec 56]  x2  via EAX,EDX
  +0x4 [dec 4]  x1  via EDX
  +0x30 [dec 48]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0040deb0 FUN_0040deb0 size=64 ret=0 callers=1 callees=2

void __fastcall FUN_0040deb0(int param_1)

{
  SetThreadPriority(*(HANDLE *)(param_1 + 0x38),0);
  SetThreadPriority(*(HANDLE *)(param_1 + 0x38),0);
  *(undefined4 *)(param_1 + 0x30) = 1;
  SetEvent(*(HANDLE *)(param_1 + 4));
  return;
}
```

### decoy: 0x0040def0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0040def0: 16-byte aligned: YES
  0040def0  55             PUSH EBP
  0040def1  8B EC          MOV EBP,ESP
  0040def3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0040def0
string literals (0):
direct callees (1):
  0x0040E170  CExoArrayList<CExoString>::SetAllocated
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
```

## Card B16-2-05

GOG function: CAurTextureBasic::RemoveFromOrderedLists  gog 0x00816030  callconv=(blank)  param_size_bytes=0  gog_size=87  gog_retn=0
Candidate Steam address: 0x00423820 (size 57, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.25) | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00423930:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00815f60 CAurTextureBasic::AddToOrderedLists -> 0x00423860  gate_class=AUTO
- GOG-order neighbour after:  0x00816090 CAurTextureBasic::Reset -> 0x00423990  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x008730c0 List_T__::Remove -> Steam ?  in candidate callees: n/a
  0x00873110 List_T__::Count -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0xd7} | Steam {0x3c,0xd0,0xd4}
imm sets (* = in both, 0 common): GOG {} | Steam {0xc,0x9c}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00423820: 16-byte aligned: YES
  00423820  55             PUSH EBP
  00423821  8B EC          MOV EBP,ESP
  00423823  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00423820
string literals (0):
direct callees (1):
  0x00423740  (unnamed)
struct offsets touched (top 3):
  +0x3C [dec 60]  x1  via EAX
  +0xD0 [dec 208]  x1  via EDX
  +0xD4 [dec 212]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00423820 FUN_00423820 size=57 ret=0 callers=2 callees=1

void __fastcall FUN_00423820(int param_1)

{
  undefined2 uVar1;
  
  uVar1 = FUN_00423740(param_1 + 0x9c,*(undefined2 *)(param_1 + 0xd0),
                       *(undefined4 *)(param_1 + 0x3c));
  *(undefined2 *)(param_1 + 0xd4) = uVar1;
  return;
}
```

### decoy: 0x00423930 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00423930: 16-byte aligned: YES
  00423930  55             PUSH EBP
  00423931  8B EC          MOV EBP,ESP
  00423933  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00423930
string literals (0):
direct callees (2):
  0x00476620  (unnamed)
  0x00449170  (unnamed)
struct offsets touched (top 1):
  +0xD7 [dec 215]  x1  via EDX
```

## Card B16-2-06

GOG function: CAurTextureBasic::Init  gog 0x00817210  callconv=(blank)  param_size_bytes=4  gog_size=1101  gog_retn=4
Candidate Steam address: 0x00424b10 (size 1101, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CAurTextureBasic::ParseTXI | G7 mac: pass(0.93) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00424f60:STR0/CALLEE0/SIG0.192/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008171c0 DirtyTextureController::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00817660 CAurFontInfo::ParseField  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (12), mapped to Steam through trusted rows only:
  0x00815da0 CAurFontInfo::Constructor -> Steam ?  in candidate callees: n/a
  0x008161b0 CAurTextureBasic::ParseField -> Steam ?  in candidate callees: n/a
  0x00867a20 Global::AurResGet -> Steam ?  in candidate callees: n/a
  0x00867e10 Global::AurResFree -> Steam ?  in candidate callees: n/a
  0x00867ee0 Global::AurResGetNextLine -> Steam ?  in candidate callees: n/a
  0x00868260 Global::AurResGetDataBytes -> Steam ?  in candidate callees: n/a
  0x0086b770 Global::AurAnisotropicFilterAvailable -> Steam ?  in candidate callees: n/a
  0x008fe2a0 List_SafePointer_WindManager___::Destructor -> Steam ?  in candidate callees: n/a
  ... 4 more GOG callees
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 8 common): GOG {0x1c*,0x34*,0x38*,0x94*,0x98*,0xa4*,0xa8*,0xdf*} | Steam {0x1c*,0x34*,0x38*,0x94*,0x98*,0xa4*,0xa8*,0xdf*}
imm sets (* = in both, 11 common): GOG {0x2*,0x4*,0x8*,0x9*,0xa*,0x10*,0x18*,0x20*,0x24*,0x30*,0x2050*} | Steam {0x2*,0x4*,0x8*,0x9*,0xa*,0x10*,0x18*,0x20*,0x24*,0x30*,0x2050*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00424b10: 16-byte aligned: YES
  00424b10  55             PUSH EBP
  00424b11  8B EC          MOV EBP,ESP
  00424b13  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00424b10
string literals (0):
direct callees (12):
  0x0091E050  (unnamed)
  0x00919723  operator new
  0x004236A0  (unnamed)
  0x00475AB0  (unnamed)
  0x00473E50  operator delete[] wrapper (`if (*p) FUN_0091c6aa(*p)`)
  0x009196FD  operator delete
  0x004762F0  ModelFile::ReadBlock(n, src)
  0x00475F70  (unnamed)
  0x00423AB0  CAurTextureBasic::ParseTXILine
  0x00475EA0  CloseModelStream
  0x004849E0  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 8):
  +0x38 [dec 56]  x8  via EAX,ECX,EDX
  +0xA8 [dec 168]  x2  via EDX
  +0x1C [dec 28]  x1  via ECX
  +0x34 [dec 52]  x1  via EDX
  +0x94 [dec 148]  x1  via EDX
  +0x98 [dec 152]  x1  via EDX
  +0xA4 [dec 164]  x1  via EAX
  +0xDF [dec 223]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00424b10 FUN_00424b10 size=1101 ret=4 callers=2 callees=12

/* WARNING: Function: __alloca_probe replaced with injection: alloca_probe */

void __thiscall FUN_00424b10(int *param_1,undefined4 param_2)

{
  char *pcVar1;
  char cVar2;
  int iVar3;
  undefined4 uVar4;
  int iVar5;
  int local_2058;
  char *local_2038;
  char local_2030 [8196];
  uint local_2c;
  char *local_28;
  int local_24;
  char *local_20;
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094907a;
  local_10 = ExceptionList;
  local_2c = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_14 = 0;
  local_18 = 0;
  cVar2 = (**(code **)(*param_1 + 0x34))(local_2c);
  if (cVar2 == '\0') {
    if (param_1[0xe = byte +0x38] == 0) {
      iVar3 = FUN_00919723(0x30);
      local_8 = 0;
      if (iVar3 == 0) {
        local_2058 = 0;
      }
      else {
        local_2058 = FUN_004236a0();
      }
      local_8 = 0xffffffff;
      param_1[0xe = byte +0x38] = local_2058;
    }
    local_1c = FUN_00475ab0(param_2,&DAT_009883e0,&local_14,1);
    if (local_1c == 0) {
      iVar3 = param_1[0xe = byte +0x38];
      if (iVar3 != 0) {
        local_8 = 1;
        FUN_00473e50();
        local_8 = 0xffffffff;
        FUN_00473e50();
        FUN_009196fd(iVar3);
      }
      param_1[0xe = byte +0x38] = 0;
      (**(code **)(*param_1 + 0xa8))(1);
      goto LAB_00424f42;
    }
    if (local_14 < 0) {
      local_18 = -1 - local_14;
    }
    else {
      local_18 = local_14;
    }
    if (0 < local_18) {
      uVar4 = FUN_004762f0(0,local_1c,local_18);
      (**(code **)(*param_1 + 0xa4))(uVar4);
      while (local_20 = (char *)FUN_00475f70(), local_20 != (char *)0x0) {
        for (; (*local_20 == ' ' || (*local_20 == '\t')); local_20 = local_20 + 1) {
        }
        FUN_00423ab0(local_20);
      }
    }
    FUN_00475ea0(local_1c,local_14);
    if ((param_1[0xe = byte +0x38] != 0) && (*(int *)(param_1[0xe = byte +0x38] + 0x1c) == 0)) {
      iVar3 = param_1[0xe = byte +0x38];
      if (iVar3 != 0) {
        local_8 = 2;
/* ... 52 more lines truncated by --max-lines */
```

### decoy: 0x00424f60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00424f60: 16-byte aligned: YES
  00424f60  55             PUSH EBP
  00424f61  8B EC          MOV EBP,ESP
  00424f63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00424f60
string literals (16):
  0x00988450  at 00424f74  'numchars'
  0x00988450  at 00424f89  'numchars'
  0x00988444  at 00424faf  'fontheight'
  0x00988444  at 00424fc7  'fontheight'
  0x00988434  at 00424fed  'baselineheight'
  0x00988434  at 00425005  'baselineheight'
  0x00988424  at 0042502b  'texturewidth'
  0x00988424  at 00425043  'texturewidth'
  0x00988418  at 00425069  'spacingR'
  0x00988418  at 00425081  'spacingR'
  0x0098840C  at 004250a7  'spacingB'
  0x0098840C  at 004250bf  'spacingB'
direct callees (6):
  0x00484D10  Global::AurMessageGetToken
  0x009370C7  _stricmp
  0x00919F10  _strlen
  0x00484E20  (unnamed)
  0x00484E00  (unnamed)
  0x00429010  (unnamed)
struct offsets touched (top 0):
```

## Card B16-2-07

GOG function: CSWGlobalVariableTable::AddIdentifierBoolean  gog 0x006512f0  callconv=(blank)  param_size_bytes=4  gog_size=88  gog_retn=4
Candidate Steam address: 0x00654490 (size 88, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWGlobalVariableTable::AddNumber | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00654430:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00651190 CSWGlobalVariableTable::AddIdentifier -> 0x006542d0  gate_class=AUTO
- GOG-order neighbour after:  0x00651350 CSWGlobalVariableTable::AddIdentifierNumber -> 0x00654490  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00651190 CSWGlobalVariableTable::AddIdentifier -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0xeb9c} | Steam {0xeba0}
imm sets (* = in both, 2 common): GOG {0x4*,0x8*,0x384} | Steam {0x4*,0x8*,0x3e8}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00654490: 16-byte aligned: YES
  00654490  55             PUSH EBP
  00654491  8B EC          MOV EBP,ESP
  00654493  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00654490
string literals (0):
direct callees (1):
  0x006542D0  (unnamed)
struct offsets touched (top 1):
  +0xEBA0 [dec 60320]  x4  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00654490 FUN_00654490 size=88 ret=4 callers=2 callees=1

undefined4 __thiscall FUN_00654490(int param_1,undefined4 param_2)

{
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  if (*(int *)(param_1 + 0xeba0) < 1000) {
    local_8 = FUN_006542d0(param_2,1,*(undefined4 *)(param_1 + 0xeba0));
    *(int *)(param_1 + 0xeba0) = *(int *)(param_1 + 0xeba0) + 1;
  }
  return local_8;
}
```

### decoy: 0x00654430 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00654430: 16-byte aligned: YES
  00654430  55             PUSH EBP
  00654431  8B EC          MOV EBP,ESP
  00654433  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00654430
string literals (0):
direct callees (1):
  0x006542D0  (unnamed)
struct offsets touched (top 1):
  +0xEB9C [dec 60316]  x4  via EAX,ECX
```

## Card B16-2-08

GOG function: CExoMoviePlayerInternal::StartMovie  gog 0x0091fe60  callconv=(blank)  param_size_bytes=4  gog_size=1975  gog_retn=4
Candidate Steam address: 0x0040cc70 (size 1975, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CExoMoviePlayerInternal::OpenClip | G7 mac: pass(0.55) | G8 mac_rotation: rotation:0x0091fbe0:0.71>0.51
- anchors: A-STR 4 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,SIG  (content level C2)
- evidence summary: STR 'Error'@00986c58; 'CExoMoviePlayerInternal::CExoMoviePlayer'@00986d28; 'SW Movie Player Window'@00986d70; SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0040d430:STR0/CALLEE0/SIG0.093/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0091fe20 CExoMoviePlayerInternal::CancelMovie -> 0x0040cc30  gate_class=REVIEW
- GOG-order neighbour after:  0x00920620 Global::ExoMoviePlayerThreadProc  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (12), mapped to Steam through trusted rows only:
  0x0040ccf0 CClientExoApp::GetClientOptions -> Steam 0x0072fb00  in candidate callees: yes
  0x0044ce60 Global::FindMovieInOverrides -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x006059e0 CExoString::operator=_2 -> Steam ?  in candidate callees: n/a
  0x00605f00 CExoString::operator+ -> Steam 0x00733df0  in candidate callees: yes
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  ... 4 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (5): 'Error'; 'CExoMoviePlayerInternal::CExoMoviePlayer'; 'SW Movie Player Window'; 'SWMovieWindow' ...
disp sets (* = in both, 11 common): GOG {0x4*,0xc*,0x30*,0x48*,0x4c*,0x50*,0x60*,0x64*,0x84*,0x88*,0x4c4*} | Steam {0x4*,0xc*,0x30*,0x48*,0x4c*,0x50*,0x60*,0x64*,0x84*,0x88*,0x4c4*}
imm sets (* = in both, 23 common): GOG {0x2*,0x4*,0x8*,0xf*,0x2e*,0x3a*,0x3c*,0x40*,0x42*,0x64*,0x65*,0x68*,0x110*,0x121*,...(+9)} | Steam {0x2*,0x4*,0x8*,0xf*,0x2e*,0x3a*,0x3c*,0x40*,0x42*,0x64*,0x65*,0x68*,0x110*,0x121*,...(+9)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0040cc70: 16-byte aligned: YES
  0040cc70  55             PUSH EBP
  0040cc71  8B EC          MOV EBP,ESP
  0040cc73  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0040cc70
string literals (5):
  0x00986D88  at 0040cd23  'SWMovieWindow'
  0x00986C58  at 0040cd64  'Error'
  0x00986D70  at 0040ce0a  'SW Movie Player Window'
  0x00986D88  at 0040ce0f  'SWMovieWindow'
  0x00986D28  at 0040d063  "CExoMoviePlayerInternal::CExoMoviePlayerInternal()  Can't open movie: "
direct callees (12):
  0x00734010  CExoString::CStr
  0x0091D5C0  (unnamed)
  0x007337C0  CExoString::operator=
  0x007A0860  (unnamed)
  0x007338D0  CExoString::operator=_2
  0x007397B0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x00733540  CExoString::Constructor
  0x00733DF0  CExoString::operator+
  0x0040CB20  (unnamed)
  0x0072FB00  CClientExoApp::GetClientOptions
  0x0091C860  _ftol2_sse
struct offsets touched (top 10):
  +0x48 [dec 72]  x13  via EAX,ECX,EDX
  +0x50 [dec 80]  x8  via EAX,ECX,EDX
  +0x4 [dec 4]  x7  via EAX,ECX,EDX
  +0x4C [dec 76]  x5  via EAX,EDX
  +0x30 [dec 48]  x4  via EAX,ECX,EDX
  +0x84 [dec 132]  x3  via EAX,ECX
  +0x88 [dec 136]  x3  via EAX,ECX,EDX
  +0x64 [dec 100]  x2  via EAX,EDX
  +0xC [dec 12]  x1  via EAX
  +0x60 [dec 96]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0040cc70 FUN_0040cc70 size=1975 ret=4 callers=1 callees=36

undefined4 __thiscall FUN_0040cc70(int param_1,undefined4 param_2)

{
  ATOM AVar1;
  uint uVar2;
  DWORD dwMessageId;
  BOOL BVar3;
  undefined4 uVar4;
  HWND pHVar5;
  char *pcVar6;
  int iVar7;
  DWORD dwLanguageId;
  LPCSTR *lpBuffer;
  DWORD nSize;
  va_list *Arguments;
  undefined4 uVar8;
  undefined1 local_cc [8];
  undefined1 local_c4 [8];
  int local_bc;
  float local_b8;
  int local_b4;
  undefined4 local_b0;
  char *local_a4;
  LPCSTR local_a0;
  char *local_9c;
  HDC local_98;
  WNDCLASSA local_94;
  tagWINDOWINFO local_6c;
  undefined4 local_30;
  int local_2c;
  int local_28;
  int local_24;
  int local_20;
  uint local_1c;
  HWND local_18;
  undefined1 local_14 [4];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009481e1;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *(undefined4 *)(param_1 + 0x50) = 0;
  DAT_00a1b9e8 = (HWND)0x0;
  if (DAT_00a1b9ec == 0) {
    local_94.style = 0;
    local_94.lpfnWndProc = FUN_0040c4a0;
    local_94.cbClsExtra = 0;
    local_94.cbWndExtra = 0;
    local_94.hInstance = DAT_00a13c6c;
    local_94.hIcon = LoadIconA(DAT_00a13c6c,(LPCSTR)0x65);
    local_94.hCursor = LoadCursorA((HINSTANCE)0x0,(LPCSTR)0x7f00);
    local_94.hbrBackground = (HBRUSH)0x0;
    local_94.lpszMenuName = (LPCSTR)0x0;
    local_94.lpszClassName = "SWMovieWindow";
    AVar1 = RegisterClassA(&local_94);
    if (AVar1 == 0) {
      Arguments = (va_list *)0x0;
      nSize = 0;
      lpBuffer = &local_a0;
      dwLanguageId = 0x400;
      dwMessageId = GetLastError();
      FormatMessageA(0x1300,(LPCVOID)0x0,dwMessageId,dwLanguageId,(LPSTR)lpBuffer,nSize,Arguments);
      MessageBoxA((HWND)0x0,local_a0,"Error",0x40);
      LocalFree(local_a0);
      ExceptionList = local_10;
      return 0;
    }
    DAT_00a1b9ec = 1;
  }
  *(undefined4 *)(param_1 + 0x30) = 1;
  local_6c.cbSize = 0x3c;
  BVar3 = GetWindowInfo(DAT_00a1b484,&local_6c);
  if (BVar3 == 0) {
    uVar4 = 0;
  }
/* ... 119 more lines truncated by --max-lines */
```

### decoy: 0x0040d430 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0040d430: 16-byte aligned: YES
  0040d430  55             PUSH EBP
  0040d431  8B EC          MOV EBP,ESP
  0040d433  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0040d430
string literals (0):
direct callees (6):
  0x00733B60  (unnamed)
  0x0040CC70  CExoMoviePlayerInternal::OpenClip (creates "SWMovieWindow" and Bink)
  0x0040D6F0  Movie::PlayFrames
  0x007338D0  CExoString::operator=_2
  0x0040DA20  Movie::EndClip
  0x0040DFE0  CExoArrayList<CExoString>::SetSize(n)
struct offsets touched (top 10):
  +0x14 [dec 20]  x4  via EAX,ECX,EDX
  +0x1C [dec 28]  x4  via ECX,EDX
  +0x9C [dec 156]  x4  via EAX
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EDX
  +0xC [dec 12]  x2  via EAX,EDX
  +0x18 [dec 24]  x2  via EAX,EDX
  +0x20 [dec 32]  x2  via EAX,ECX
  +0x94 [dec 148]  x2  via ECX,EDX
  +0x30 [dec 48]  x1  via ECX
```

## Card B16-2-09

GOG function: CSWGuiTextParams::ReSetFont  gog 0x009151e0  callconv=(blank)  param_size_bytes=0  gog_size=19  gog_retn=0
Candidate Steam address: 0x00416e10 (size 19, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00416de0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x009151b0 CSWGuiTextParams::SetBaseFont -> 0x00416de0  gate_class=REVIEW
- GOG-order neighbour after:  0x00915200 CSWGuiTextParams::SetText -> 0x00416e30  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00915130 CSWGuiTextParams::SetFontRename -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 9): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x00416e10: 16-byte aligned: YES
  00416e10  55             PUSH EBP
  00416e11  8B EC          MOV EBP,ESP
  00416e13  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00416e10
string literals (0):
direct callees (1):
  0x00416D60  CSWGuiTextParams::SetFontRename
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00416e10 FUN_00416e10 size=19 ret=0 callers=9 callees=1
THUNK -> 0x00416D60 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_00416e10(undefined4 param_1)

{
  FUN_00416d60(param_1);
  return;
}
```

### decoy: 0x00416de0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00416de0: 16-byte aligned: YES
  00416de0  55             PUSH EBP
  00416de1  8B EC          MOV EBP,ESP
  00416de3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00416de0
string literals (0):
direct callees (2):
  0x00710980  (unnamed)
  0x00416D60  CSWGuiTextParams::SetFontRename
struct offsets touched (top 0):
```

## Card B16-2-10

GOG function: CServerAIMaster::ClearEventQueue  gog 0x0064da00  callconv=(blank)  param_size_bytes=0  gog_size=91  gog_retn=0
Candidate Steam address: 0x0051e350 (size 1593, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.08 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051e220:STR0/CALLEE0/SIG0.367/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064d1b0 CServerAIMaster::UpdateState -> 0x0051d790  gate_class=AUTO
- GOG-order neighbour after:  0x0064da60 CServerAIMaster::AddObject -> 0x0051e040  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00477c30 unnamed -> Steam ?  in candidate callees: n/a
  0x00650700 CServerAIMaster::DeleteEventData -> Steam ?  in candidate callees: n/a
  0x007c0300 CExoLinkedList_CAurObject_::RemoveHead -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x007b4960 CServerExoAppInternal::UnloadModule -> Steam 0x005315e0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x10,0x14} | Steam {}
imm sets (* = in both, 2 common): GOG {0x4*,0xc*,0x54} | Steam {0x2,0x3,0x4*,0x5,0x6,0xa,0xc*,0x1a,0x20,0x26,0x48}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051e350: 16-byte aligned: YES
  0051e350  55             PUSH EBP
  0051e351  8B EC          MOV EBP,ESP
  0051e353  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0051e350
string literals (67):
  0x00992D78  at 0051e417  'EVENT_TIMED_EVENT'
  0x00992D60  at 0051e429  'EVENT_ENTERED_TRIGGER'
  0x00992D4C  at 0051e43b  'EVENT_LEFT_TRIGGER'
  0x00992D34  at 0051e44d  'EVENT_REMOVE_FROM_AREA'
  0x00992D20  at 0051e45f  'EVENT_APPLY_EFFECT'
  0x00992D0C  at 0051e471  'EVENT_CLOSE_OBJECT'
  0x00992CF8  at 0051e483  'EVENT_OPEN_OBJECT'
  0x00992CE4  at 0051e495  'EVENT_SPELL_IMPACT'
  0x00992CCC  at 0051e4a7  'EVENT_PLAY_ANIMATION'
  0x00992CB8  at 0051e4b9  'EVENT_SIGNAL_EVENT'
  0x00992CA0  at 0051e4cb  'EVENT_DESTROY_OBJECT'
  0x00992C8C  at 0051e4dd  'EVENT_UNLOCK_OBJECT'
direct callees (6):
  0x00733540  CExoString::Constructor
  0x0051EAA0  (unnamed)
  0x007338D0  CExoString::operator=_2
  0x00734270  CExoString::Format
  0x00734010  CExoString::CStr
  0x00733780  CExoString::~CExoString
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0051e350 FUN_0051e350 size=1593 ret=0 callers=1 callees=6

void FUN_0051e350(undefined4 param_1,undefined4 param_2,int param_3,undefined2 *param_4)

{
  undefined4 uVar1;
  undefined1 local_48 [8];
  undefined1 local_40 [8];
  undefined1 local_38 [8];
  undefined1 local_30 [8];
  undefined1 local_28 [8];
  undefined1 local_20 [8];
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094d828;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  FUN_00733540();
  local_8._0_1_ = 1;
  FUN_00733540();
  local_8._0_1_ = 2;
  FUN_00733540();
  local_8._0_1_ = 3;
  FUN_00733540();
  local_8._0_1_ = 4;
  FUN_00733540();
  local_8._0_1_ = 5;
  FUN_00733540();
  local_8 = CONCAT31(local_8._1_3_,6);
  FUN_0051eaa0(param_2,local_18,local_28);
  FUN_0051eaa0(param_1,local_40,local_38);
  switch(param_3) {
  case 1:
    FUN_007338d0("EVENT_TIMED_EVENT");
    break;
  case 2:
    FUN_007338d0("EVENT_ENTERED_TRIGGER");
    break;
  case 3:
    FUN_007338d0("EVENT_LEFT_TRIGGER");
    break;
  case 4:
    FUN_007338d0("EVENT_REMOVE_FROM_AREA");
    break;
  case 5:
    FUN_007338d0("EVENT_APPLY_EFFECT");
    break;
  case 6:
    FUN_007338d0("EVENT_CLOSE_OBJECT");
    break;
  case 7:
    FUN_007338d0("EVENT_OPEN_OBJECT");
    break;
  case 8:
    FUN_007338d0("EVENT_SPELL_IMPACT");
    break;
  case 9:
    FUN_007338d0("EVENT_PLAY_ANIMATION");
    break;
  case 10:
    FUN_007338d0("EVENT_SIGNAL_EVENT");
    break;
  case 0xb:
    FUN_007338d0("EVENT_DESTROY_OBJECT");
    break;
  case 0xc:
    FUN_007338d0("EVENT_UNLOCK_OBJECT");
    break;
  case 0xd:
    FUN_007338d0("EVENT_LOCK_OBJECT");
    break;
  case 0xe:
    FUN_007338d0("EVENT_REMOVE_EFFECT");
    break;
  case 0xf:
/* ... 183 more lines truncated by --max-lines */
```

### decoy: 0x0051e220 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051e220: 16-byte aligned: YES
  0051e220  55             PUSH EBP
  0051e221  8B EC          MOV EBP,ESP
  0051e223  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051e220
string literals (0):
direct callees (8):
  0x0051E350  (unnamed)
  0x00919723  operator new
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x0051AFE0  CWorldTimer::CompareWorldTimes
  0x005210D0  (unnamed)
  0x0058C370  (unnamed)
  0x0054AA30  (unnamed)
struct offsets touched (top 6):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x8 [dec 8]  x1  via EAX
  +0xC [dec 12]  x1  via EDX
  +0x10 [dec 16]  x1  via ECX
  +0x14 [dec 20]  x1  via EAX
  +0x10048 [dec 65608]  x1  via ECX
```

## Card B16-2-11

GOG function: CSWGuiDialogLetterbox::`scalar  gog 0x005af1f0  callconv=(blank)  param_size_bytes=4  gog_size=114  gog_retn=4
Candidate Steam address: 0x008ba980 (size 275, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CSWGuiDialogLetterbox::Constructor | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x008baaa0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005af0d0 CSWGuiDialogLetterbox::Constructor -> 0x008ba980  gate_class=AUTO
- GOG-order neighbour after:  0x005af270 CSWGuiDialogLetterbox::SetExtentSpecial -> 0x008bab20  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
  0x0090cce0 CSWGuiPanel::Destructor -> Steam 0x0040e860  in candidate callees: no
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8,0xc,0x88,0x8c}
imm sets (* = in both, 2 common): GOG {0x4*,0x90*} | Steam {0x4*,0xc,0x54,0x68,0x78,0x90*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x008ba980: 16-byte aligned: YES
  008ba980  55             PUSH EBP
  008ba981  8B EC          MOV EBP,ESP
  008ba983  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x008ba980
string literals (0):
direct callees (3):
  0x0040E5A0  CSWGuiPanel::Constructor
  0x00733540  CExoString::Constructor
  0x008BAFD0  CSWGuiDialogLetterbox::ResetFade
struct offsets touched (top 5):
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
  +0x8 [dec 8]  x3  via EAX,ECX
  +0xC [dec 12]  x2  via EAX,EDX
  +0x88 [dec 136]  x1  via EDX
  +0x8C [dec 140]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 008ba980 FUN_008ba980 size=275 ret=4 callers=1 callees=3

undefined4 * __thiscall FUN_008ba980(undefined4 *param_1,undefined4 param_2)

{
  uint uVar1;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00979d26;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_0040e5a0(param_2);
  local_8 = 0;
  *param_1 = CSWGuiDialogLetterbox::vftable;
  param_1[0x1d = byte +0x74] = 0;
  param_1[0x1c = byte +0x70] = 0;
  param_1[0x1b = byte +0x6c] = 0;
  param_1[0x1a = byte +0x68] = 0;
  param_1[0x21 = byte +0x84] = 0;
  param_1[0x20 = byte +0x80] = 0;
  param_1[0x1f = byte +0x7c] = 0;
  param_1[0x1e = byte +0x78] = 0;
  FUN_00733540(uVar1);
  local_8 = CONCAT31(local_8._1_3_,1);
  param_1[0x15 = byte +0x54] = DAT_00a7fd34;
  param_1[0x16 = byte +0x58] = DAT_00a7fd38;
  param_1[0x17 = byte +0x5c] = DAT_00a7fd3c;
  FUN_008bafd0();
  param_1[0x22 = byte +0x88] = 0;
  param_1[0x23 = byte +0x8c] = 0;
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x008baaa0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008baaa0: 16-byte aligned: YES
  008baaa0  55             PUSH EBP
  008baaa1  8B EC          MOV EBP,ESP
  008baaa3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x008baaa0
string literals (0):
direct callees (3):
  0x00733780  CExoString::~CExoString
  0x0040E860  CSWGuiPanel::Destructor
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card B16-2-12

GOG function: CSWGuiDialogLetterbox::`scalar  gog 0x005af1f0  callconv=(blank)  param_size_bytes=4  gog_size=114  gog_retn=4
Candidate Steam address: 0x008bac30 (size 110, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CSWGuiDialogLetterbox::SetExtent | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.12 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x008bab20:STR0/CALLEE0/SIG0.083/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005af0d0 CSWGuiDialogLetterbox::Constructor -> 0x008ba980  gate_class=AUTO
- GOG-order neighbour after:  0x005af270 CSWGuiDialogLetterbox::SetExtentSpecial -> 0x008bab20  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
  0x0090cce0 CSWGuiPanel::Destructor -> Steam 0x0040e860  in candidate callees: no
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8,0xc,0x1c,0x6e,0x88}
imm sets (* = in both, 1 common): GOG {0x4*,0x90} | Steam {0x2,0x4*,0x1c}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x008bac30: 16-byte aligned: YES
  008bac30  55             PUSH EBP
  008bac31  8B EC          MOV EBP,ESP
  008bac33  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008bac30
string literals (0):
direct callees (1):
  0x0040E980  CSWGuiPanel::SetExtent
struct offsets touched (top 6):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0xC [dec 12]  x1  via EAX
  +0x1C [dec 28]  x1  via EDX
  +0x6E [dec 110]  x1  via ECX
  +0x88 [dec 136]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 008bac30 FUN_008bac30 size=110 ret=4 callers=0 callees=1

void __thiscall FUN_008bac30(int param_1,undefined4 *param_2)

{
  undefined4 local_14;
  int local_10;
  undefined4 local_c;
  int local_8;
  
  local_14 = *param_2;
  local_10 = param_2[1 = byte +0x4];
  local_c = param_2[2 = byte +0x8];
  local_8 = param_2[3 = byte +0xc];
  if (*(int *)(param_1 + 0x88) == 1) {
    local_10 = 0;
  }
  else if (*(int *)(param_1 + 0x88) == 2) {
    local_8 = *(short *)(*(int *)(param_1 + 0x1c) + 0x6e) - local_10;
  }
  FUN_0040e980(&local_14);
  return;
}
```

### decoy: 0x008bab20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008bab20: 16-byte aligned: YES
  008bab20  55             PUSH EBP
  008bab21  8B EC          MOV EBP,ESP
  008bab23  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008bab20
string literals (0):
direct callees (0):
struct offsets touched (top 9):
  +0x6C [dec 108]  x4  via EAX,ECX,EDX
  +0x74 [dec 116]  x4  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX
  +0x8 [dec 8]  x2  via EAX
  +0xC [dec 12]  x2  via EAX
  +0x88 [dec 136]  x2  via EAX
  +0x1C [dec 28]  x1  via EAX
  +0x6E [dec 110]  x1  via EDX
  +0x8C [dec 140]  x1  via EDX
```

## Card B16-2-13

GOG function: CAurTextureBasic::GetController  gog 0x00815aa0  callconv=(blank)  param_size_bytes=0  gog_size=17  gog_retn=0
Candidate Steam address: 0x004233a0 (size 17, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00423000:STR0/CALLEE0/SIG0.008/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00815700 CAurTextureBasic::Constructor -> 0x00423000  gate_class=AUTO
- GOG-order neighbour after:  0x00815ac0 Global::CAurTextureBasic_scalar_deleting_destructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x34*} | Steam {0x34*}
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
prologue for 0x004233a0: 16-byte aligned: YES
  004233a0  55             PUSH EBP
  004233a1  8B EC          MOV EBP,ESP
  004233a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004233a0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x34 [dec 52]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 004233a0 FUN_004233a0 size=17 ret=0 callers=0 callees=0

undefined4 __fastcall FUN_004233a0(int param_1)

{
  return *(undefined4 *)(param_1 + 0x34);
}
```

### decoy: 0x00423000 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00423000: 16-byte aligned: YES
  00423000  55             PUSH EBP
  00423001  8B EC          MOV EBP,ESP
  00423003  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00423000
string literals (0):
direct callees (6):
  0x00421E30  CAurTexture::Constructor
  0x0091CFF0  (unnamed)
  0x00919723  operator new
  0x004236A0  (unnamed)
  0x00423820  (unnamed)
  0x00423860  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x18 [dec 24]  x1  via ECX
  +0x1C [dec 28]  x1  via EDX
  +0x20 [dec 32]  x1  via EAX
  +0x24 [dec 36]  x1  via EAX
  +0x28 [dec 40]  x1  via ECX
  +0x2C [dec 44]  x1  via EDX
  +0x30 [dec 48]  x1  via EAX
  +0x34 [dec 52]  x1  via EDX
```

## Card B16-2-14

GOG function: CExoMoviePlayerInternal::HideFadePanel  gog 0x00920e40  callconv=(blank)  param_size_bytes=0  gog_size=183  gog_retn=0
Candidate Steam address: 0x0040dc50 (size 183, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0040dac0:STR0/CALLEE0/SIG0.525/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00920cb0 CSWGuiSaveLoad::HandleCloudButton2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00920f00 CExoMoviePlayerInternal::UpdateMovieWindow -> 0x0040dd10  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0040d380 CClientExoApp::GetSWGuiManager -> Steam ?  in candidate callees: n/a
  0x00610d50 CExoCriticalSection::EnterCriticalSection -> Steam ?  in candidate callees: n/a
  0x00610d70 CExoCriticalSection::LeaveCriticalSection -> Steam ?  in candidate callees: n/a
  0x0090eaf0 CSWGuiManager::RemovePanel -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x44*,0x50*} | Steam {0x4*,0x44*,0x50*}
imm sets (* = in both, 4 common): GOG {0x5*,0x18*,0x40*,0x41*} | Steam {0x5*,0x18*,0x40*,0x41*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0040dc50: 16-byte aligned: YES
  0040dc50  55             PUSH EBP
  0040dc51  8B EC          MOV EBP,ESP
  0040dc53  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0040dc50
string literals (0):
direct callees (4):
  0x00739CF0  (unnamed)
  0x0073FEA0  CClientExoApp::GetSWGuiManager
  0x00410670  (unnamed)
  0x00739D10  (unnamed)
struct offsets touched (top 3):
  +0x44 [dec 68]  x3  via EAX,ECX
  +0x4 [dec 4]  x1  via ECX
  +0x50 [dec 80]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0040dc50 FUN_0040dc50 size=183 ret=0 callers=1 callees=4

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0040dc50(int param_1)

{
  undefined4 uVar1;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  
  FUN_00739cf0();
  if (*(int *)(param_1 + 0x44) != 0) {
    if (_DAT_00986b50 <= 0.0) {
      local_14 = 1.0;
    }
    else {
      local_14 = 0.0;
    }
    if (local_14 <= (float)_DAT_00986b38) {
      local_1c = 0;
    }
    else {
      if (_DAT_00986b50 <= 0.0) {
        local_18 = 0x3f800000;
      }
      else {
        local_18 = 0;
      }
      local_1c = local_18;
    }
    *(undefined4 *)(*(int *)(param_1 + 0x44) + 0x50) = local_1c;
    uVar1 = *(undefined4 *)(param_1 + 0x44);
    FUN_0073fea0(uVar1);
    FUN_00410670(uVar1);
  }
  FUN_00739d10();
  return;
}
```

### decoy: 0x0040dac0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0040dac0: 16-byte aligned: YES
  0040dac0  55             PUSH EBP
  0040dac1  8B EC          MOV EBP,ESP
  0040dac3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0040dac0
string literals (0):
direct callees (7):
  0x00739CF0  (unnamed)
  0x00919723  operator new
  0x0073FEA0  CClientExoApp::GetSWGuiManager
  0x007BC600  (unnamed)
  0x00410530  CSWGuiManager::AddPanel
  0x00409ED0  (unnamed)
  0x00739D10  (unnamed)
struct offsets touched (top 4):
  +0x44 [dec 68]  x5  via EAX,ECX,EDX
  +0x4 [dec 4]  x3  via ECX,EDX
  +0x8 [dec 8]  x1  via EDX
  +0x50 [dec 80]  x1  via ECX
```

## Card B16-2-15

GOG function: CSWGuiTextParams::operator=  gog 0x00914fa0  callconv=(blank)  param_size_bytes=4  gog_size=235  gog_retn=4
Candidate Steam address: 0x00416bd0 (size 235, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00416aa0:STR0/CALLEE1/SIG0.773/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00914e70 CSWGuiTextParams::Constructor_2 -> 0x00416aa0  gate_class=REVIEW
- GOG-order neighbour after:  0x00915090 CSWGuiTextParams::SetFont -> 0x00416cc0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x00915440 CSWGuiTextParams::SetTextObject -> Steam ?  in candidate callees: n/a
  0x00924c20 Global::strcpy -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 8): the card's A-CALLEE credit may come from these, not from callees
  0x005a4df0 CSWGuiInGameItemEntry::SetItem -> Steam 0x008b1250  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x4*,0x8*,0x28*,0x38*,0x50*} | Steam {0x4*,0x8*,0x28*,0x38*,0x50*}
imm sets (* = in both, 10 common): GOG {0x4*,0x6*,0x8*,0x14*,0x1c*,0x2c*,0x3c*,0x3f*,0xffffffbf*,0xffffffc0*} | Steam {0x4*,0x6*,0x8*,0x14*,0x1c*,0x2c*,0x3c*,0x3f*,0xffffffbf*,0xffffffc0*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00416bd0: 16-byte aligned: YES
  00416bd0  55             PUSH EBP
  00416bd1  8B EC          MOV EBP,ESP
  00416bd3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00416bd0
string literals (0):
direct callees (3):
  0x007337C0  CExoString::operator=
  0x0091CFF0  (unnamed)
  0x00417070  (unnamed)
struct offsets touched (top 5):
  +0x8 [dec 8]  x6  via EAX,ECX,EDX
  +0x38 [dec 56]  x6  via EAX,ECX,EDX
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x28 [dec 40]  x2  via EAX,ECX
  +0x50 [dec 80]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00416bd0 FUN_00416bd0 size=235 ret=4 callers=8 callees=3

int __thiscall FUN_00416bd0(int param_1,int param_2)

{
  FUN_007337c0(param_2);
  *(undefined4 *)(param_1 + 8) = *(undefined4 *)(param_2 + 8);
  FUN_0091cff0(param_1 + 0x3c,param_2 + 0x3c);
  *(undefined4 *)(param_1 + 0x2c) = *(undefined4 *)(param_2 + 0x1c);
  *(undefined4 *)(param_1 + 0x30) = *(undefined4 *)(param_2 + 0x20);
  *(undefined4 *)(param_1 + 0x34) = *(undefined4 *)(param_2 + 0x24);
  *(undefined4 *)(param_1 + 0x1c) = *(undefined4 *)(param_1 + 0x2c);
  *(undefined4 *)(param_1 + 0x20) = *(undefined4 *)(param_1 + 0x30);
  *(undefined4 *)(param_1 + 0x24) = *(undefined4 *)(param_1 + 0x34);
  *(uint *)(param_1 + 0x38) =
       *(uint *)(param_1 + 0x38) & 0xffffffc0 | *(uint *)(param_2 + 0x38) & 0x3f;
  *(undefined4 *)(param_1 + 0x28) = *(undefined4 *)(param_2 + 0x28);
  *(uint *)(param_1 + 0x38) =
       *(uint *)(param_1 + 0x38) & 0xffffffbf | (*(uint *)(param_2 + 0x38) >> 6 & 1) << 6;
  FUN_00417070(*(undefined4 *)(param_1 + 0x50));
  return param_1;
}
```

### decoy: 0x00416aa0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00416aa0: 16-byte aligned: YES
  00416aa0  55             PUSH EBP
  00416aa1  8B EC          MOV EBP,ESP
  00416aa3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00416aa0
string literals (0):
direct callees (4):
  0x00733540  CExoString::Constructor
  0x007107F0  (unnamed)
  0x007337C0  CExoString::operator=
  0x00416DE0  CSWGuiTextParams::SetBaseFont
struct offsets touched (top 5):
  +0x8 [dec 8]  x7  via EAX,ECX,EDX
  +0x4 [dec 4]  x6  via EAX,ECX,EDX
  +0x38 [dec 56]  x4  via EAX,ECX,EDX
  +0x28 [dec 40]  x1  via EAX
  +0x50 [dec 80]  x1  via EDX
```

## Card B16-2-16

GOG function: CExoMoviePlayerInternal::UpdateMovieWindow  gog 0x00920f00  callconv=(blank)  param_size_bytes=0  gog_size=245  gog_retn=4
Candidate Steam address: 0x0040dd10 (size 245, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=movie | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0040dc50:STR0/CALLEE0/SIG0.083/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00920e40 CExoMoviePlayerInternal::HideFadePanel -> 0x0040dc50  gate_class=REVIEW
- GOG-order neighbour after:  0x00921000 CExoMoviePlayerInternal::PauseMovie -> 0x0040de50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4c*,0x50*,0x84*,0x88*} | Steam {0x4c*,0x50*,0x84*,0x88*}
imm sets (* = in both, 5 common): GOG {0x4*,0x8*,0x3c*,0x42*,0x5c*} | Steam {0x4*,0x8*,0x3c*,0x42*,0x5c*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 4)
```

### prologue
```
prologue for 0x0040dd10: 16-byte aligned: YES
  0040dd10  55             PUSH EBP
  0040dd11  8B EC          MOV EBP,ESP
  0040dd13  83 EC 5C       SUB ESP,0x5c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0040dd10
string literals (0):
direct callees (0):
struct offsets touched (top 4):
  +0x4C [dec 76]  x2  via EAX,EDX
  +0x50 [dec 80]  x2  via ECX,EDX
  +0x84 [dec 132]  x1  via EAX
  +0x88 [dec 136]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0040dd10 FUN_0040dd10 size=245 ret=4 callers=1 callees=8

void __fastcall FUN_0040dd10(int param_1)

{
  BOOL BVar1;
  HDC hdc;
  tagWINDOWINFO local_50;
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  
  if (*(int *)(param_1 + 0x4c) != 0) {
    _BinkBufferSetOffset_12 [GHIDRA-AUTO-LABEL: unreliable]
              (*(undefined4 *)(param_1 + 0x4c),*(undefined4 *)(param_1 + 0x84),
               *(undefined4 *)(param_1 + 0x88));
  }
  local_50.cbSize = 0x3c;
  BVar1 = GetWindowInfo(DAT_00a1b484,&local_50);
  if (BVar1 != 0) {
    local_14 = local_50.rcClient.left;
    local_10 = local_50.rcClient.top;
    local_c = local_50.rcClient.right;
    local_8 = local_50.rcClient.bottom;
    BVar1 = MoveWindow(*(HWND *)(param_1 + 0x50),local_50.rcClient.left,local_50.rcClient.top,
                       local_50.rcClient.right - local_50.rcClient.left,
                       local_50.rcClient.bottom - local_50.rcClient.top,1);
    if (BVar1 == 0) {
      GetLastError();
    }
    hdc = GetDC(DAT_00a1b484);
    PatBlt(hdc,0,0,DAT_009f42a4,DAT_009f42a8,0x42);
    ReleaseDC(DAT_00a1b484,hdc);
    ShowWindow(*(HWND *)(param_1 + 0x50),8);
  }
  return;
}
```

### decoy: 0x0040dc50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0040dc50: 16-byte aligned: YES
  0040dc50  55             PUSH EBP
  0040dc51  8B EC          MOV EBP,ESP
  0040dc53  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0040dc50
string literals (0):
direct callees (4):
  0x00739CF0  (unnamed)
  0x0073FEA0  CClientExoApp::GetSWGuiManager
  0x00410670  (unnamed)
  0x00739D10  (unnamed)
struct offsets touched (top 3):
  +0x44 [dec 68]  x3  via EAX,ECX
  +0x4 [dec 4]  x1  via ECX
  +0x50 [dec 80]  x1  via EAX
```

## Card B16-2-17

GOG function: CExoMoviePlayerInternal::RequestRestart  gog 0x0091fdf0  callconv=(blank)  param_size_bytes=4  gog_size=47  gog_retn=4
Candidate Steam address: 0x0040cc00 (size 47, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Movie::NoteDeactivated | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0040cc70:STR0/CALLEE0/SIG0.022/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0091fd10 CExoMoviePlayerInternal::ShutDown -> 0x0040cb20  gate_class=REVIEW
- GOG-order neighbour after:  0x0091fe20 CExoMoviePlayerInternal::CancelMovie -> 0x0040cc30  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x1c*,0x20*} | Steam {0x1c*,0x20*}
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
prologue for 0x0040cc00: 16-byte aligned: YES
  0040cc00  55             PUSH EBP
  0040cc01  8B EC          MOV EBP,ESP
  0040cc03  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0040cc00
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x20 [dec 32]  x2  via ECX,EDX
  +0x1C [dec 28]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0040cc00 FUN_0040cc00 size=47 ret=4 callers=1 callees=0

void __thiscall FUN_0040cc00(int param_1,int param_2)

{
  *(undefined4 *)(param_1 + 0x1c) = 1;
  if ((*(int *)(param_1 + 0x20) == 0) || (param_2 == 1)) {
    *(int *)(param_1 + 0x20) = param_2;
  }
  return;
}
```

### decoy: 0x0040cc70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0040cc70: 16-byte aligned: YES
  0040cc70  55             PUSH EBP
  0040cc71  8B EC          MOV EBP,ESP
  0040cc73  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0040cc70
string literals (5):
  0x00986D88  at 0040cd23  'SWMovieWindow'
  0x00986C58  at 0040cd64  'Error'
  0x00986D70  at 0040ce0a  'SW Movie Player Window'
  0x00986D88  at 0040ce0f  'SWMovieWindow'
  0x00986D28  at 0040d063  "CExoMoviePlayerInternal::CExoMoviePlayerInternal()  Can't open movie: "
direct callees (12):
  0x00734010  CExoString::CStr
  0x0091D5C0  (unnamed)
  0x007337C0  CExoString::operator=
  0x007A0860  (unnamed)
  0x007338D0  CExoString::operator=_2
  0x007397B0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x00733540  CExoString::Constructor
  0x00733DF0  CExoString::operator+
  0x0040CB20  (unnamed)
  0x0072FB00  CClientExoApp::GetClientOptions
  0x0091C860  _ftol2_sse
struct offsets touched (top 10):
  +0x48 [dec 72]  x13  via EAX,ECX,EDX
  +0x50 [dec 80]  x8  via EAX,ECX,EDX
  +0x4 [dec 4]  x7  via EAX,ECX,EDX
  +0x4C [dec 76]  x5  via EAX,EDX
  +0x30 [dec 48]  x4  via EAX,ECX,EDX
  +0x84 [dec 132]  x3  via EAX,ECX
  +0x88 [dec 136]  x3  via EAX,ECX,EDX
  +0x64 [dec 100]  x2  via EAX,EDX
  +0xC [dec 12]  x1  via EAX
  +0x60 [dec 96]  x1  via ECX
```

## Card B16-2-18

GOG function: CSWGuiInGameCharacter::OnPanelRemoved  gog 0x005991e0  callconv=(blank)  param_size_bytes=0  gog_size=32  gog_retn=0
Candidate Steam address: 0x0084fc00 (size 200, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0084fbb0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00599070 CSWGuiInGameCharacter::SetModText -> 0x0084f770  gate_class=REVIEW
- GOG-order neighbour after:  0x00599200 CSWGuiInGameCharacter::HandleInputEvent -> 0x0084f900  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0090d940 CSWGuiPanel::OnPanelRemoved -> Steam 0x0040f4c0  in candidate callees: no
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4b70} | Steam {0x4,0x8,0x48,0x80,0x94,0xd8,0x3c98,0x3c9c,0x4b6c}
imm sets (* = in both, 0 common): GOG {} | Steam {0x24,0x3c,0x3ea}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0084fc00: 16-byte aligned: YES
  0084fc00  55             PUSH EBP
  0084fc01  8B EC          MOV EBP,ESP
  0084fc03  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0084fc00
string literals (0):
direct callees (0):
struct offsets touched (top 9):
  +0x4B6C [dec 19308]  x4  via EAX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0x48 [dec 72]  x1  via ECX
  +0x80 [dec 128]  x1  via EDX
  +0x94 [dec 148]  x1  via EDX
  +0xD8 [dec 216]  x1  via EAX
  +0x3C98 [dec 15512]  x1  via EAX
  +0x3C9C [dec 15516]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0084fc00 FUN_0084fc00 size=200 ret=0 callers=3 callees=0

void __fastcall FUN_0084fc00(int param_1)

{
  int iVar1;
  float10 fVar2;
  
  if (DAT_009f42b4 != 0) {
    (**(code **)(**(int **)(param_1 + 0x4b6c) + 0x94))(*(undefined4 *)(param_1 + 0x3c98));
    iVar1 = (**(code **)(**(int **)(param_1 + 0x3c9c) + 0x80))(0x3ea);
    if (iVar1 != 0) {
      *(undefined4 *)(iVar1 + 0x48) = 1;
      fVar2 = (float10)(**(code **)(**(int **)(param_1 + 0x4b6c) + 0xd8))();
      *(undefined4 *)(iVar1 + 0x3c) = 0;
      *(undefined4 *)(iVar1 + 0x40) = 0;
      *(float *)(iVar1 + 0x44) = (float)fVar2;
    }
  }
  return;
}
```

### decoy: 0x0084fbb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0084fbb0: 16-byte aligned: YES
  0084fbb0  55             PUSH EBP
  0084fbb1  8B EC          MOV EBP,ESP
  0084fbb3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0084fbb0
string literals (0):
direct callees (3):
  0x0084E6F0  CSWGuiInGameCharacter::SetStats (CanLevelUp at 0x84F221 -> button visibility)
  0x0041B1D0  (unnamed)
  0x0040EC90  CSWGuiPanel::Draw
struct offsets touched (top 1):
  +0x3AD8 [dec 15064]  x1  via EAX
```

## Card B16-2-19

GOG function: CAurTextureBasic::CanDownSampleFurther  gog 0x00818ff0  callconv=(blank)  param_size_bytes=0  gog_size=107  gog_retn=0
Candidate Steam address: 0x004268f0 (size 107, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CAurTextureBasic::IsDownSampleCandidate | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00426960:STR0/CALLEE0/SIG0.417/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00818f40 CAurTextureBasic::SetScheme -> 0x00426840  gate_class=AUTO
- GOG-order neighbour after:  0x00819060 CAurTextureBasic::SetNewDownSampleLevel -> 0x00426960  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x68*,0x6c*,0xbc*,0xc0*,0xc2*} | Steam {0x68*,0x6c*,0xbc*,0xc0*,0xc2*}
imm sets (* = in both, 2 common): GOG {0x2*,0x8*} | Steam {0x2*,0x8*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x004268f0: 16-byte aligned: YES
  004268f0  55             PUSH EBP
  004268f1  8B EC          MOV EBP,ESP
  004268f3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004268f0
string literals (0):
direct callees (0):
struct offsets touched (top 5):
  +0xC0 [dec 192]  x2  via EAX,ECX
  +0x68 [dec 104]  x1  via EDX
  +0x6C [dec 108]  x1  via ECX
  +0xBC [dec 188]  x1  via EAX
  +0xC2 [dec 194]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 004268f0 FUN_004268f0 size=107 ret=0 callers=1 callees=0

undefined4 __fastcall FUN_004268f0(int param_1)

{
  undefined4 uVar1;
  byte bVar2;
  
  bVar2 = (char)*(undefined2 *)(param_1 + 0xc0) - (char)*(undefined2 *)(param_1 + 0xc2);
  if (*(short *)(param_1 + 0xc0) < *(short *)(param_1 + 0xbc)) {
    if ((*(int *)(param_1 + 0x68) >> (bVar2 & 0x1f) < 3) ||
       (*(int *)(param_1 + 0x6c) >> (bVar2 & 0x1f) < 3)) {
      uVar1 = 0;
    }
    else {
      uVar1 = 1;
    }
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}
```

### decoy: 0x00426960 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00426960: 16-byte aligned: YES
  00426960  55             PUSH EBP
  00426961  8B EC          MOV EBP,ESP
  00426963  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00426960
string literals (0):
direct callees (1):
  0x00474000  UniquePush(list ECX, ptr)
struct offsets touched (top 4):
  +0xC0 [dec 192]  x4  via EAX,ECX,EDX
  +0xBC [dec 188]  x2  via EAX,EDX
  +0xC2 [dec 194]  x1  via EAX
  +0xE8 [dec 232]  x1  via EDX
```

## Card B16-2-20

GOG function: CExoMoviePlayerInternal::CancelMovie  gog 0x0091fe20  callconv=(blank)  param_size_bytes=8  gog_size=59  gog_retn=8
Candidate Steam address: 0x0040cc30 (size 59, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Movie::Stop | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0040cc70:STR0/CALLEE0/SIG0.057/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0091fdf0 CExoMoviePlayerInternal::RequestRestart -> 0x0040cc00  gate_class=REVIEW
- GOG-order neighbour after:  0x0091fe60 CExoMoviePlayerInternal::StartMovie -> 0x0040cc70  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x14*,0x18*,0x30*,0x9c*} | Steam {0x14*,0x18*,0x30*,0x9c*}
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
prologue for 0x0040cc30: 16-byte aligned: YES
  0040cc30  55             PUSH EBP
  0040cc31  8B EC          MOV EBP,ESP
  0040cc33  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0040cc30
string literals (0):
direct callees (0):
struct offsets touched (top 4):
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
  +0x30 [dec 48]  x1  via EAX
  +0x9C [dec 156]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0040cc30 FUN_0040cc30 size=59 ret=8 callers=2 callees=0

void __thiscall FUN_0040cc30(int param_1,undefined4 param_2,int param_3)

{
  if ((param_3 != 0) || ((*(int *)(param_1 + 0x30) == 1 && (*(int *)(param_1 + 0x9c) != 0)))) {
    *(undefined4 *)(param_1 + 0x14) = 1;
  }
  *(undefined4 *)(param_1 + 0x18) = param_2;
  return;
}
```

### decoy: 0x0040cc70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0040cc70: 16-byte aligned: YES
  0040cc70  55             PUSH EBP
  0040cc71  8B EC          MOV EBP,ESP
  0040cc73  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0040cc70
string literals (5):
  0x00986D88  at 0040cd23  'SWMovieWindow'
  0x00986C58  at 0040cd64  'Error'
  0x00986D70  at 0040ce0a  'SW Movie Player Window'
  0x00986D88  at 0040ce0f  'SWMovieWindow'
  0x00986D28  at 0040d063  "CExoMoviePlayerInternal::CExoMoviePlayerInternal()  Can't open movie: "
direct callees (12):
  0x00734010  CExoString::CStr
  0x0091D5C0  (unnamed)
  0x007337C0  CExoString::operator=
  0x007A0860  (unnamed)
  0x007338D0  CExoString::operator=_2
  0x007397B0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x00733540  CExoString::Constructor
  0x00733DF0  CExoString::operator+
  0x0040CB20  (unnamed)
  0x0072FB00  CClientExoApp::GetClientOptions
  0x0091C860  _ftol2_sse
struct offsets touched (top 10):
  +0x48 [dec 72]  x13  via EAX,ECX,EDX
  +0x50 [dec 80]  x8  via EAX,ECX,EDX
  +0x4 [dec 4]  x7  via EAX,ECX,EDX
  +0x4C [dec 76]  x5  via EAX,EDX
  +0x30 [dec 48]  x4  via EAX,ECX,EDX
  +0x84 [dec 132]  x3  via EAX,ECX
  +0x88 [dec 136]  x3  via EAX,ECX,EDX
  +0x64 [dec 100]  x2  via EAX,EDX
  +0xC [dec 12]  x1  via EAX
  +0x60 [dec 96]  x1  via ECX
```

## Card B16-2-21

GOG function: CExoMoviePlayerInternal::PlayMoviesAsync  gog 0x00920c30  callconv=(blank)  param_size_bytes=12  gog_size=120  gog_retn=12
Candidate Steam address: 0x0040da40 (size 120, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.75) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0040da20:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00920c10 CExoMoviePlayerInternal::StopMovie -> 0x0040dac0  gate_class=REVIEW
- GOG-order neighbour after:  0x00920cb0 CSWGuiSaveLoad::HandleCloudButton2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x005ac200 unnamed -> Steam ?  in candidate callees: n/a
  0x00633790 CERFString::Write; -> Steam ?  in candidate callees: n/a
  0x006fcef0 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x30*,0x38*,0x8c*} | Steam {0x4*,0x30*,0x38*,0x8c*}
imm sets (* = in both, 3 common): GOG {0x8*,0xc*,0x90*} | Steam {0x8*,0xc*,0x90*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x0040da40: 16-byte aligned: YES
  0040da40  55             PUSH EBP
  0040da41  8B EC          MOV EBP,ESP
  0040da43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0040da40
string literals (0):
direct callees (3):
  0x0040DF70  (unnamed)
  0x0040E120  (unnamed)
  0x006169E0  (unnamed)
struct offsets touched (top 4):
  +0x4 [dec 4]  x1  via ECX
  +0x30 [dec 48]  x1  via EAX
  +0x38 [dec 56]  x1  via EAX
  +0x8C [dec 140]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0040da40 FUN_0040da40 size=120 ret=12 callers=1 callees=5

void __thiscall FUN_0040da40(int param_1,undefined4 param_2,undefined4 param_3,int param_4)

{
  SetThreadPriority(*(HANDLE *)(param_1 + 0x38),0);
  FUN_0040df70(param_2);
  *(undefined4 *)(param_1 + 0x8c) = param_3;
  if (param_4 == 0) {
    FUN_006169e0(0);
  }
  else {
    FUN_0040e120(param_4);
  }
  *(undefined4 *)(param_1 + 0x30) = 1;
  SetEvent(*(HANDLE *)(param_1 + 4));
  return;
}
```

### decoy: 0x0040da20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0040da20: 16-byte aligned: YES
  0040da20  55             PUSH EBP
  0040da21  8B EC          MOV EBP,ESP
  0040da23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0040da20
string literals (0):
direct callees (1):
  0x0040CB20  (unnamed)
struct offsets touched (top 1):
  +0x30 [dec 48]  x1  via EAX
```

## Card B16-2-22

GOG function: CSWGlobalVariableTable::AddIdentifierBoolean  gog 0x006512f0  callconv=(blank)  param_size_bytes=4  gog_size=88  gog_retn=4
Candidate Steam address: 0x00654210 (size 178, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.14 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00653e40:STR0/CALLEE0/SIG0.077/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00651190 CSWGlobalVariableTable::AddIdentifier -> 0x006542d0  gate_class=AUTO
- GOG-order neighbour after:  0x00651350 CSWGlobalVariableTable::AddIdentifierNumber -> 0x00654490  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00651190 CSWGlobalVariableTable::AddIdentifier -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0xeb9c} | Steam {0xebac}
imm sets (* = in both, 2 common): GOG {0x4*,0x8*,0x384} | Steam {0x4*,0x8*,0x1c,0xff,0x93d,0xffffff}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00654210: 16-byte aligned: YES
  00654210  55             PUSH EBP
  00654211  8B EC          MOV EBP,ESP
  00654213  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00654210
string literals (0):
direct callees (3):
  0x00919F10  _strlen
  0x00733DB0  CExoString::operator[]
  0x00920830  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00654210 FUN_00654210 size=178 ret=4 callers=2 callees=3

uint __thiscall FUN_00654210(int param_1,int *param_2)

{
  char cVar1;
  uint uVar2;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_8;
  
  local_18 = 0;
  local_8 = 0;
  while( true ) {
    if (*param_2 == 0) {
      local_1c = 0;
    }
    else {
      local_1c = _strlen [GHIDRA-AUTO-LABEL: unreliable]((char *)*param_2);
    }
    if ((int)local_1c <= local_8) break;
    cVar1 = FUN_00733db0(local_8);
    uVar2 = _toupper [GHIDRA-AUTO-LABEL: unreliable]((int)cVar1);
    local_18 = local_18 >> 8 ^ *(uint *)(param_1 + 0xebac + ((local_18 ^ uVar2) & 0xff) * 4);
    local_8 = local_8 + 1;
  }
  return local_18 % 0x93d;
}
```

### decoy: 0x00653e40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00653e40: 16-byte aligned: YES
  00653e40  55             PUSH EBP
  00653e41  8B EC          MOV EBP,ESP
  00653e43  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00653e40
string literals (8):
  0x009984D0  at 00654007  'BOOLEAN'
  0x009984C8  at 00654036  'NUMBER'
  0x009984BC  at 00654062  'LOCATION'
  0x009984B4  at 0065408e  'STRING'
  0x00998488  at 006540d6  "Script var '%s' has bad type in catalogue!"
  0x0099845C  at 00654122  "Script var '%s' '%s' won't fit in table!"
  0x00998430  at 0065415f  "Script var '%s' duplicated in catalogue!"
  0x00998408  at 0065419e  "Script var '%s' longer than %d letters!"
direct callees (17):
  0x0071AD40  CRes2DA helper ctor (name, flag)
  0x00733540  CExoString::Constructor
  0x0071C3D0  C2DA::LoadText
  0x00733570  CExoString::CExoString_char
  0x0071B1D0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x00919F10  _strlen
  0x006545B0  GlobalVars Find thiscall(table, CExoString* name) RET 4 (AddBoolean 0x654430 has no dup check)
  0x00734F60  CExoString::CompareNoCase(char*)
  0x00654430  CSWGlobalVariableTable::AddBoolean(name)
  0x00654490  CSWGlobalVariableTable::AddNumber(name)
  0x006544F0  (unnamed)
  0x00654550  CSWGlobalVariableTable::AddString(name)
  0x00734010  CExoString::CStr
  0x00734270  CExoString::Format
  0x0071AE80  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0xC [dec 12]  x2  via EAX,ECX
```

## Card B16-2-23

GOG function: CSWGuiTextParams::Constructor_2  gog 0x00914e70  callconv=(blank)  param_size_bytes=20  gog_size=301  gog_retn=20
Candidate Steam address: 0x00416aa0 (size 301, retn 20)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00416bd0:STR0/CALLEE1/SIG0.773/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00914d30 CSWGuiTextParams::Constructor -> 0x00416960  gate_class=AUTO
- GOG-order neighbour after:  0x00914fa0 CSWGuiTextParams::operator= -> 0x00416bd0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x0061a140 CResRef::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x009151b0 CSWGuiTextParams::SetBaseFont -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 4): the card's A-CALLEE credit may come from these, not from callees
  0x005a4df0 CSWGuiInGameItemEntry::SetItem -> Steam 0x008b1250  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x4*,0x8*,0x28*,0x38*,0x50*} | Steam {0x4*,0x8*,0x28*,0x38*,0x50*}
imm sets (* = in both, 7 common): GOG {0xc*,0x14*,0x1c*,0x2c*,0x3f*,0xffffffbf*,0xffffffc0*} | Steam {0xc*,0x14*,0x1c*,0x2c*,0x3f*,0xffffffbf*,0xffffffc0*}

### abi
```
GOG param bytes: 20; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(20/4)*4 = 20 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: TRUE (GOG 20 bytes -> x86 slots 20; ASM RET 20)
```

### prologue
```
prologue for 0x00416aa0: 16-byte aligned: YES
  00416aa0  55             PUSH EBP
  00416aa1  8B EC          MOV EBP,ESP
  00416aa3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00416aa0
string literals (0):
direct callees (4):
  0x00733540  CExoString::Constructor
  0x007107F0  (unnamed)
  0x007337C0  CExoString::operator=
  0x00416DE0  CSWGuiTextParams::SetBaseFont
struct offsets touched (top 5):
  +0x8 [dec 8]  x7  via EAX,ECX,EDX
  +0x4 [dec 4]  x6  via EAX,ECX,EDX
  +0x38 [dec 56]  x4  via EAX,ECX,EDX
  +0x28 [dec 40]  x1  via EAX
  +0x50 [dec 80]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00416aa0 FUN_00416aa0 size=301 ret=20 callers=4 callees=4

int __thiscall
FUN_00416aa0(int param_1,undefined4 param_2,undefined4 param_3,undefined4 *param_4,uint param_5,
            undefined4 param_6)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009486c8;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  FUN_007107f0();
  *(undefined4 *)(param_1 + 0x1c) = 0;
  *(undefined4 *)(param_1 + 0x20) = 0;
  *(undefined4 *)(param_1 + 0x24) = 0;
  *(undefined4 *)(param_1 + 0x2c) = 0;
  *(undefined4 *)(param_1 + 0x30) = 0;
  *(undefined4 *)(param_1 + 0x34) = 0;
  *(undefined4 *)(param_1 + 0x50) = 0;
  FUN_007337c0(param_2);
  *(undefined4 *)(param_1 + 0x2c) = *param_4;
  *(undefined4 *)(param_1 + 0x30) = param_4[1 = byte +0x4];
  *(undefined4 *)(param_1 + 0x34) = param_4[2 = byte +0x8];
  FUN_00416de0(param_3);
  *(undefined4 *)(param_1 + 0x1c) = *(undefined4 *)(param_1 + 0x2c);
  *(undefined4 *)(param_1 + 0x20) = *(undefined4 *)(param_1 + 0x30);
  *(undefined4 *)(param_1 + 0x24) = *(undefined4 *)(param_1 + 0x34);
  *(uint *)(param_1 + 0x38) = *(uint *)(param_1 + 0x38) & 0xffffffc0 | param_5 & 0x3f;
  *(undefined4 *)(param_1 + 8) = 0xffffffff;
  *(undefined4 *)(param_1 + 0x28) = param_6;
  *(uint *)(param_1 + 0x38) = *(uint *)(param_1 + 0x38) & 0xffffffbf;
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x00416bd0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00416bd0: 16-byte aligned: YES
  00416bd0  55             PUSH EBP
  00416bd1  8B EC          MOV EBP,ESP
  00416bd3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00416bd0
string literals (0):
direct callees (3):
  0x007337C0  CExoString::operator=
  0x0091CFF0  (unnamed)
  0x00417070  (unnamed)
struct offsets touched (top 5):
  +0x8 [dec 8]  x6  via EAX,ECX,EDX
  +0x38 [dec 56]  x6  via EAX,ECX,EDX
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x28 [dec 40]  x2  via EAX,ECX
  +0x50 [dec 80]  x1  via EDX
```

## Card B16-2-24

GOG function: CAurTextureBasic::ApplyDownSampleLevel  gog 0x00817fb0  callconv=(blank)  param_size_bytes=0  gog_size=18  gog_retn=0
Candidate Steam address: 0x004258b0 (size 18, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: BuildAndStoreTexture 0x4260e0)
- best decoy (neighbour/other candidate) scores: 0x00425800:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00817f00 CAurTextureBasic::LoadImagePLT -> 0x00425800  gate_class=AUTO
- GOG-order neighbour after:  0x008184b0 CAurTextureBasic::GetMemoryUsage -> 0x00425db0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x008187e0 CAurTextureBasic::BuildAndStoreTexture -> Steam 0x004260e0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x48*} | Steam {0x48*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x004258b0: 16-byte aligned: YES
  004258b0  55             PUSH EBP
  004258b1  8B EC          MOV EBP,ESP
  004258b3  83 EC 48       SUB ESP,0x48
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004258b0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 004258b0 FUN_004258b0 size=18 ret=0 callers=2 callees=0

void FUN_004258b0(void)

{
  return;
}
```

### decoy: 0x00425800 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00425800: 16-byte aligned: YES
  00425800  55             PUSH EBP
  00425801  8B EC          MOV EBP,ESP
  00425803  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00425800
string literals (0):
direct callees (2):
  0x0091CFF0  (unnamed)
  0x00421BC0  (unnamed)
struct offsets touched (top 5):
  +0x44 [dec 68]  x3  via EAX,ECX
  +0xE7 [dec 231]  x2  via ECX,EDX
  +0x78 [dec 120]  x1  via EDX
  +0x9C [dec 156]  x1  via ECX
  +0xB4 [dec 180]  x1  via EDX
```

## Card B16-2-25

GOG function: CSWGuiTextParams::SetDefaultColor_2  gog 0x00915550  callconv=(blank)  param_size_bytes=0  gog_size=26  gog_retn=0
Candidate Steam address: 0x00417180 (size 26, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00417010:STR0/CALLEE1/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00915510 CSWGuiTextParams::SetDefaultColor -> 0x00417140  gate_class=AUTO
- GOG-order neighbour after:  0x00915570 CSWGuiText::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x009153e0 CSWGuiTextParams::SetColor -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 6): the card's A-CALLEE credit may come from these, not from callees
  0x00918530 CSWGuiButton::SetActive -> Steam 0x0041a290  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x2c*} | Steam {0x2c*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00417180: 16-byte aligned: YES
  00417180  55             PUSH EBP
  00417181  8B EC          MOV EBP,ESP
  00417183  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00417180
string literals (0):
direct callees (1):
  0x00417010  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00417180 FUN_00417180 size=26 ret=0 callers=6 callees=1
THUNK -> 0x00417010 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_00417180(int param_1)

{
  FUN_00417010(param_1 + 0x2c);
  return;
}
```

### decoy: 0x00417010 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00417010: 16-byte aligned: YES
  00417010  55             PUSH EBP
  00417011  8B EC          MOV EBP,ESP
  00417013  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00417010
string literals (0):
direct callees (0):
struct offsets touched (top 5):
  +0x50 [dec 80]  x4  via ECX,EDX
  +0x14 [dec 20]  x3  via EAX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0xC [dec 12]  x1  via EAX
```

## Card B16-2-26

GOG function: CSWGlobalVariableTable::Load  gog 0x006535d0  callconv=(blank)  param_size_bytes=0  gog_size=108  gog_retn=0
Candidate Steam address: 0x00656780 (size 31, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00656710:STR1/CALLEE2/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00653500 CSWGlobalVariableTable::Save -> 0x006564c0  gate_class=REVIEW
- GOG-order neighbour after:  0x00653640 CSWSCreature::AIActionPhysicalAttacks -> 0x006d7390  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: no
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
  0x00653380 CSWGlobalVariableTable::ReadTable -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x005d1d20 CSWGuiSaveLoad::LoadPCAutoSave -> Steam 0x008529e0  in candidate callers: no
  0x007b2f00 CServerExoAppInternal::LoadGame -> Steam 0x0052fb80  in candidate callers: no
GOG strings (1): 'GLOBALVARS'
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 0 common): GOG {0x10} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00656780: 16-byte aligned: YES
  00656780  55             PUSH EBP
  00656781  8B EC          MOV EBP,ESP
  00656783  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00656780
string literals (0):
direct callees (1):
  0x005E2D10  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00656780 FUN_00656780 size=31 ret=0 callers=1 callees=1

undefined4 * __fastcall FUN_00656780(undefined4 *param_1)

{
  FUN_005e2d10();
  *param_1 = CSWSMessage::vftable;
  return param_1;
}
```

### decoy: 0x00656710 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00656710: 16-byte aligned: YES
  00656710  55             PUSH EBP
  00656711  8B EC          MOV EBP,ESP
  00656713  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00656710
string literals (1):
  0x0099874C  at 00656738  'GLOBALVARS'
direct callees (3):
  0x00733570  CExoString::CExoString_char
  0x006564C0  (unnamed)
  0x00733780  CExoString::~CExoString
struct offsets touched (top 0):
```

## Card B16-2-27

GOG function: CExoMoviePlayerInternal::ResumeMovie  gog 0x00921040  callconv=(blank)  param_size_bytes=0  gog_size=59  gog_retn=0
Candidate Steam address: 0x0040de50 (size 59, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0040de10:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00921000 CExoMoviePlayerInternal::PauseMovie -> 0x0040de50  gate_class=REVIEW
- GOG-order neighbour after:  0x00921080 CExoMoviePlayerInternal::RestartMoviesAsync -> 0x0040deb0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x48*,0x64*} | Steam {0x48*,0x64*}
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
prologue for 0x0040de50: 16-byte aligned: YES
  0040de50  55             PUSH EBP
  0040de51  8B EC          MOV EBP,ESP
  0040de53  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0040de50
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x64 [dec 100]  x3  via EAX,ECX,EDX
  +0x48 [dec 72]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0040de50 FUN_0040de50 size=59 ret=0 callers=1 callees=1

void __fastcall FUN_0040de50(int param_1)

{
  if ((*(int *)(param_1 + 0x48) != 0) && (*(int *)(param_1 + 0x64 [dec 100]) == 1)) {
    *(undefined4 *)(param_1 + 0x64 [dec 100]) = 0;
    _BinkPause_8 [GHIDRA-AUTO-LABEL: unreliable](*(undefined4 *)(param_1 + 0x48),*(undefined4 *)(param_1 + 0x64 [dec 100]));
  }
  return;
}
```

### decoy: 0x0040de10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0040de10: 16-byte aligned: YES
  0040de10  55             PUSH EBP
  0040de11  8B EC          MOV EBP,ESP
  0040de13  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0040de10
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x64 [dec 100]  x3  via EAX,ECX,EDX
  +0x48 [dec 72]  x2  via EAX,EDX
```

## Card B16-2-28

GOG function: CSWGuiInGameGameplay::OnExitDiff  gog 0x005e6080  callconv=(blank)  param_size_bytes=0  gog_size=70  gog_retn=4
Candidate Steam address: 0x008e1570 (size 94, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.92 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x008e15d0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x005e6020 CSWGuiInGameGameplay::OnEnterDiff -> 0x008e1570  gate_class=REVIEW
- GOG-order neighbour after:  0x005e60d0 CSWGuiInGameGameplay::SetUpOptions -> 0x008e1620  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x009153e0 CSWGuiTextParams::SetColor -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x38*} | Steam {0x38*}
imm sets (* = in both, 5 common): GOG {0x4*,0x6*,0x8*,0xea0*,0xffffffbf*} | Steam {0x4*,0x6*,0x8*,0xea0*,0xa519,0xffffffbf*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 4)
```

### prologue
```
prologue for 0x008e1570: 16-byte aligned: YES
  008e1570  55             PUSH EBP
  008e1571  8B EC          MOV EBP,ESP
  008e1573  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008e1570
string literals (0):
direct callees (3):
  0x00417010  (unnamed)
  0x008E1280  (unnamed)
  0x008E1410  (unnamed)
struct offsets touched (top 1):
  +0x38 [dec 56]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 008e1570 FUN_008e1570 size=94 ret=4 callers=0 callees=3

void __fastcall FUN_008e1570(int param_1)

{
  FUN_00417010(&DAT_00a7fdac);
  *(uint *)(param_1 + 0xed8) = *(uint *)(param_1 + 0xed8) & 0xffffffbf | 0x40;
  FUN_008e1280();
  FUN_008e1410(0xa519);
  return;
}
```

### decoy: 0x008e15d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008e15d0: 16-byte aligned: YES
  008e15d0  55             PUSH EBP
  008e15d1  8B EC          MOV EBP,ESP
  008e15d3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008e15d0
string literals (0):
direct callees (1):
  0x00417010  (unnamed)
struct offsets touched (top 1):
  +0x38 [dec 56]  x2  via ECX,EDX
```

## Card B16-2-29

GOG function: CAurTextureBasic::Destructor  gog 0x00815b80  callconv=(blank)  param_size_bytes=0  gog_size=543  gog_retn=0
Candidate Steam address: 0x004233a0 (size 17, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.06 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00423480:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00815af0 CAurTexture::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00815da0 CAurFontInfo::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x00815af0 CAurTexture::Destructor -> Steam ?  in candidate callees: n/a
  0x00816030 CAurTextureBasic::RemoveFromOrderedLists -> Steam ?  in candidate callees: n/a
  0x00817860 CAurTextureBasic::FreeImage -> Steam 0x00425160  in candidate callees: no
  0x008184b0 CAurTextureBasic::GetMemoryUsage -> Steam ?  in candidate callees: n/a
  0x008730c0 List_T__::Remove -> Steam ?  in candidate callees: n/a
  0x00873110 List_T__::Count -> Steam ?  in candidate callees: n/a
  0x008fe2a0 List_SafePointer_WindManager___::Destructor -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  ... 1 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x34*,0x38,0x3c,0x40,0x48,0x58,0xc6,0xc8,0xd6} | Steam {0x34*}
imm sets (* = in both, 0 common): GOG {0x4,0x18,0x24,0x44} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x004233a0: 16-byte aligned: YES
  004233a0  55             PUSH EBP
  004233a1  8B EC          MOV EBP,ESP
  004233a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004233a0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x34 [dec 52]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 004233a0 FUN_004233a0 size=17 ret=0 callers=0 callees=0

undefined4 __fastcall FUN_004233a0(int param_1)

{
  return *(undefined4 *)(param_1 + 0x34);
}
```

### decoy: 0x00423480 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00423480: 16-byte aligned: YES
  00423480  55             PUSH EBP
  00423481  8B EC          MOV EBP,ESP
  00423483  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00423480
string literals (0):
direct callees (9):
  0x00473E50  operator delete[] wrapper (`if (*p) FUN_0091c6aa(*p)`)
  0x009196FD  operator delete
  0x0091C6AA  operator delete[]
  0x00425DB0  CAurTextureBasic::GetMemoryUsage
  0x00425160  CAurTextureBasic::FreeImage
  0x00423930  TextureLists Remove
  0x00476620  (unnamed)
  0x00449170  (unnamed)
  0x004233F0  CAurTexture (proxy) dtor
struct offsets touched (top 9):
  +0x40 [dec 64]  x4  via EAX,ECX
  +0x48 [dec 72]  x3  via EAX,EDX
  +0x34 [dec 52]  x1  via ECX
  +0x38 [dec 56]  x1  via ECX
  +0x3C [dec 60]  x1  via ECX
  +0x58 [dec 88]  x1  via ECX
  +0xC6 [dec 198]  x1  via EDX
  +0xC8 [dec 200]  x1  via EAX
  +0xD6 [dec 214]  x1  via ECX
```

## Card B16-2-30

GOG function: CExoMoviePlayerInternal::Constructor  gog 0x0091fa70  callconv=(blank)  param_size_bytes=0  gog_size=360  gog_retn=0
Candidate Steam address: 0x0040c880 (size 360, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0040c9f0:STR0/CALLEE0/SIG0.433/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0091f690 Global::ExoMoviePlayerWindowProc  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0091fbe0 CExoMoviePlayerInternal::Destructor -> 0x0040c9f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0044dd60 CExoArrayList_CExoString_::CExoArrayList_2 -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00567c10 CExoArrayList_int_::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x006059e0 CExoString::operator=_2 -> Steam ?  in candidate callees: n/a
  0x00610c70 CExoCriticalSection::Constructor -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 15 common): GOG {0x4*,0x14*,0x18*,0x1c*,0x20*,0x24*,0x30*,0x34*,0x38*,0x44*,0x48*,0x4c*,0x50*,0x64*,...(+1)} | Steam {0x4*,0x14*,0x18*,0x1c*,0x20*,0x24*,0x30*,0x34*,0x38*,0x44*,0x48*,0x4c*,0x50*,0x64*,...(+1)}
imm sets (* = in both, 10 common): GOG {0x2*,0x3*,0x4*,0x8*,0x28*,0x3c*,0x40*,0x68*,0x90*,0x8000*} | Steam {0x2*,0x3*,0x4*,0x8*,0x28*,0x3c*,0x40*,0x68*,0x90*,0x8000*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0040c880: 16-byte aligned: YES
  0040c880  55             PUSH EBP
  0040c881  8B EC          MOV EBP,ESP
  0040c883  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0040c880
string literals (0):
direct callees (5):
  0x0040DEF0  (unnamed)
  0x00733540  CExoString::Constructor
  0x00739C10  (unnamed)
  0x0084A640  (unnamed)
  0x007338D0  CExoString::operator=_2
struct offsets touched (top 10):
  +0x4 [dec 4]  x1  via EDX
  +0x14 [dec 20]  x1  via ECX
  +0x18 [dec 24]  x1  via EDX
  +0x1C [dec 28]  x1  via EAX
  +0x20 [dec 32]  x1  via ECX
  +0x24 [dec 36]  x1  via EDX
  +0x30 [dec 48]  x1  via EAX
  +0x34 [dec 52]  x1  via EAX
  +0x38 [dec 56]  x1  via EDX
  +0x44 [dec 68]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0040c880 FUN_0040c880 size=360 ret=0 callers=1 callees=7

undefined4 * __fastcall FUN_0040c880(undefined4 *param_1)

{
  uint uVar1;
  HANDLE pvVar2;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094813a;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_0040def0(0);
  local_8 = 0;
  FUN_00733540(uVar1);
  local_8._0_1_ = 1;
  FUN_00739c10();
  local_8._0_1_ = 2;
  FUN_00733540();
  local_8._0_1_ = 3;
  FUN_0084a640(0);
  local_8 = CONCAT31(local_8._1_3_,4);
  FUN_007338d0(&DAT_00986925);
  param_1[0xc = byte +0x30] = 0;
  *param_1 = 0;
  pvVar2 = CreateEventA((LPSECURITY_ATTRIBUTES)0x0,0,0,(LPCSTR)0x0);
  param_1[1 = byte +0x4] = pvVar2;
  pvVar2 = CreateThread((LPSECURITY_ATTRIBUTES)0x0,0x8000,FUN_0040d430,param_1,0,param_1 + 0xf);
  param_1[0xe = byte +0x38] = pvVar2;
  param_1[0xd = byte +0x34] = 0;
  param_1[5 = byte +0x14] = 0;
  param_1[6 = byte +0x18] = 0;
  param_1[7 = byte +0x1c] = 0;
  param_1[8 = byte +0x20] = 0;
  param_1[9 = byte +0x24] = 0;
  param_1[0x11 = byte +0x44] = 0;
  param_1[0x13 = byte +0x4c] = 0;
  param_1[0x12 = byte +0x48] = 0;
  param_1[0x1c = byte +0x70] = 0;
  param_1[0x14 = byte +0x50] = 0;
  param_1[0x19 = byte +0x64] = 0;
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x0040c9f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0040c9f0: 16-byte aligned: YES
  0040c9f0  55             PUSH EBP
  0040c9f1  8B EC          MOV EBP,ESP
  0040c9f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0040c9f0
string literals (0):
direct callees (5):
  0x0040CB20  (unnamed)
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x00733780  CExoString::~CExoString
  0x00739C90  (unnamed)
  0x0040DF30  (unnamed)
struct offsets touched (top 3):
  +0x44 [dec 68]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x38 [dec 56]  x2  via EAX,ECX
```

## Card B16-2-31

GOG function: CServerAIMaster::AppendToScriptsRun  gog 0x0077e650  callconv=(blank)  param_size_bytes=4  gog_size=192  gog_retn=4
Candidate Steam address: 0x0064e910 (size 493, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.15 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0064e710:STR0/CALLEE0/SIG0.062/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0077e600 CSWVirtualMachineCommands::RunScriptCallback -> 0x00669020  trusted
- GOG-order neighbour after:  0x0077e710 CSWVirtualMachineCommands::CopyGameDefinedStructure -> 0x00669130  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: no
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: no
  0x00605f00 CExoString::operator+ -> Steam 0x00733df0  in candidate callees: no
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0077e600 CSWVirtualMachineCommands::RunScriptCallback -> Steam 0x00669020  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x88,0x8c,0x90,0x9c,0xa0,0xac,0xb0,0xd4}
imm sets (* = in both, 3 common): GOG {0x2*,0x4*,0x2c*,0x60} | Steam {0x2*,0x3,0x4*,0x8,0xc,0x10,0x2c*,0x88,0xbc}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0064e910: 16-byte aligned: YES
  0064e910  55             PUSH EBP
  0064e911  8B EC          MOV EBP,ESP
  0064e913  81 EC BC 00 00 00 SUB ESP,0xbc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0064e910
string literals (0):
direct callees (10):
  0x005EA490  (unnamed)
  0x00734010  CExoString::CStr
  0x0091CAEB  (unnamed)
  0x005EA7B0  (unnamed)
  0x0091F3EF  (unnamed)
  0x0091A825  (unnamed)
  0x009205FC  (unnamed)
  0x0091A338  (unnamed)
  0x0091C5B3  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 8):
  +0x88 [dec 136]  x1  via EDX
  +0x8C [dec 140]  x1  via ECX
  +0x90 [dec 144]  x1  via ECX
  +0x9C [dec 156]  x1  via EAX
  +0xA0 [dec 160]  x1  via EAX
  +0xAC [dec 172]  x1  via EAX
  +0xB0 [dec 176]  x1  via ECX
  +0xD4 [dec 212]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0064e910 FUN_0064e910 size=493 ret=4 callers=0 callees=10

void __thiscall FUN_0064e910(int param_1,undefined4 param_2)

{
  char *_Filename [GHIDRA-AUTO-LABEL: unreliable];
  char *_Mode [GHIDRA-AUTO-LABEL: unreliable];
  undefined1 local_9c [100];
  size_t local_38;
  long local_34;
  undefined4 local_30;
  int local_2c;
  long local_28;
  size_t local_24;
  long local_20;
  size_t local_1c;
  long local_18;
  uint local_10;
  int local_c;
  FILE *local_8;
  
  local_10 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  local_c = FUN_005ea490(param_2);
  if (local_c == 1) {
    _Mode [GHIDRA-AUTO-LABEL: unreliable] = "rb+";
    _Filename [GHIDRA-AUTO-LABEL: unreliable] = (char *)FUN_00734010();
    local_8 = _fopen [GHIDRA-AUTO-LABEL: unreliable](_Filename [GHIDRA-AUTO-LABEL: unreliable],_Mode [GHIDRA-AUTO-LABEL: unreliable]);
    FUN_005ea7b0();
    _fread [GHIDRA-AUTO-LABEL: unreliable](local_9c,0x88,1,local_8);
    _fseek [GHIDRA-AUTO-LABEL: unreliable](local_8,0,2);
    local_38 = *(size_t *)(param_1 + 0xb0);
    local_34 = _ftell [GHIDRA-AUTO-LABEL: unreliable](local_8);
    local_30 = *(undefined4 *)(param_1 + 0xd4);
    _fwrite [GHIDRA-AUTO-LABEL: unreliable](*(void **)(param_1 + 0xac),0x2c,local_38,local_8);
    local_28 = _ftell [GHIDRA-AUTO-LABEL: unreliable](local_8);
    _fwrite [GHIDRA-AUTO-LABEL: unreliable](*(void **)(param_1 + 0x88),4,local_2c * 3,local_8);
    local_24 = *(size_t *)(param_1 + 0x90);
    local_20 = _ftell [GHIDRA-AUTO-LABEL: unreliable](local_8);
    _fwrite [GHIDRA-AUTO-LABEL: unreliable](*(void **)(param_1 + 0x8c),8,local_24,local_8);
    local_1c = *(size_t *)(param_1 + 0xa0);
    local_18 = _ftell [GHIDRA-AUTO-LABEL: unreliable](local_8);
    _fwrite [GHIDRA-AUTO-LABEL: unreliable](*(void **)(param_1 + 0x9c),4,local_1c,local_8);
    _fseek [GHIDRA-AUTO-LABEL: unreliable](local_8,0,0);
    _fwrite [GHIDRA-AUTO-LABEL: unreliable](local_9c,0x88,1,local_8);
    _fclose [GHIDRA-AUTO-LABEL: unreliable](local_8);
  }
  __security_check_cookie [GHIDRA-AUTO-LABEL: unreliable](local_10 ^ (uint)&stack0xfffffffc);
  return;
}
```

### decoy: 0x0064e710 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0064e710: 16-byte aligned: YES
  0064e710  55             PUSH EBP
  0064e711  8B EC          MOV EBP,ESP
  0064e713  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0064e710
string literals (0):
direct callees (1):
  0x005EA190  Global::AurPartTriggerDestroy
struct offsets touched (top 10):
  +0x34 [dec 52]  x16  via EAX,ECX,EDX
  +0x4 [dec 4]  x6  via EAX
  +0x30 [dec 48]  x4  via EAX,ECX
  +0x8 [dec 8]  x3  via EDX
  +0xC [dec 12]  x3  via ECX
  +0x64 [dec 100]  x1  via ECX
  +0x68 [dec 104]  x1  via EDX
  +0x6C [dec 108]  x1  via ECX
  +0x74 [dec 116]  x1  via EAX
  +0x78 [dec 120]  x1  via ECX
```

## Card B16-2-32

GOG function: CExoMoviePlayerInternal::PauseMovie  gog 0x00921000  callconv=(blank)  param_size_bytes=0  gog_size=59  gog_retn=0
Candidate Steam address: 0x0040de50 (size 59, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0040de10:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00920f00 CExoMoviePlayerInternal::UpdateMovieWindow -> 0x0040dd10  gate_class=REVIEW
- GOG-order neighbour after:  0x00921040 CExoMoviePlayerInternal::ResumeMovie -> 0x0040de50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x48*,0x64*} | Steam {0x48*,0x64*}
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
prologue for 0x0040de50: 16-byte aligned: YES
  0040de50  55             PUSH EBP
  0040de51  8B EC          MOV EBP,ESP
  0040de53  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0040de50
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x64 [dec 100]  x3  via EAX,ECX,EDX
  +0x48 [dec 72]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0040de50 FUN_0040de50 size=59 ret=0 callers=1 callees=1

void __fastcall FUN_0040de50(int param_1)

{
  if ((*(int *)(param_1 + 0x48) != 0) && (*(int *)(param_1 + 0x64 [dec 100]) == 1)) {
    *(undefined4 *)(param_1 + 0x64 [dec 100]) = 0;
    _BinkPause_8 [GHIDRA-AUTO-LABEL: unreliable](*(undefined4 *)(param_1 + 0x48),*(undefined4 *)(param_1 + 0x64 [dec 100]));
  }
  return;
}
```

### decoy: 0x0040de10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0040de10: 16-byte aligned: YES
  0040de10  55             PUSH EBP
  0040de11  8B EC          MOV EBP,ESP
  0040de13  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0040de10
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x64 [dec 100]  x3  via EAX,ECX,EDX
  +0x48 [dec 72]  x2  via EAX,EDX
```

## Card B16-2-33

GOG function: CExoMoviePlayerInternal::RunMovieFrameLoop  gog 0x009208e0  callconv=(blank)  param_size_bytes=0  gog_size=807  gog_retn=4
Candidate Steam address: 0x0040d6f0 (size 807, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Movie::PlayFrames | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0040d430:STR0/CALLEE0/SIG0.195/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00920620 Global::ExoMoviePlayerThreadProc  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00920c10 CExoMoviePlayerInternal::StopMovie -> 0x0040dac0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0060b670 CExoArrayList_T_::Destructor -> Steam ?  in candidate callees: n/a
  0x00616610 CExoInput::ClearEvents -> Steam ?  in candidate callees: n/a
  0x006166a0 CExoInput::GetEvents -> Steam ?  in candidate callees: n/a
  0x00921040 CExoMoviePlayerInternal::ResumeMovie -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 9 common): GOG {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x48*,0x4c*,0x64*} | Steam {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x48*,0x4c*,0x64*}
imm sets (* = in both, 8 common): GOG {0x4*,0x5*,0x10*,0x11*,0x34*,0x4d*,0x4e*,0xd4*} | Steam {0x4*,0x5*,0x10*,0x11*,0x34*,0x4d*,0x4e*,0xd4*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 4)
```

### prologue
```
prologue for 0x0040d6f0: 16-byte aligned: YES
  0040d6f0  55             PUSH EBP
  0040d6f1  8B EC          MOV EBP,ESP
  0040d6f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0040d6f0
string literals (0):
direct callees (4):
  0x0040DE50  (unnamed)
  0x0072FA80  (unnamed)
  0x0072F9F0  (unnamed)
  0x0064DDB0  Free(ptr*) delete-and-zero helper
struct offsets touched (top 9):
  +0x48 [dec 72]  x10  via EAX,ECX,EDX
  +0x4C [dec 76]  x8  via EAX,ECX,EDX
  +0x10 [dec 16]  x3  via EAX,EDX
  +0x14 [dec 20]  x3  via ECX,EDX
  +0x64 [dec 100]  x3  via EAX,ECX,EDX
  +0xC [dec 12]  x2  via EAX,EDX
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via EAX
  +0x18 [dec 24]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0040d6f0 FUN_0040d6f0 size=807 ret=4 callers=1 callees=18

undefined4 __fastcall FUN_0040d6f0(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  BOOL BVar3;
  int local_c8;
  int local_c4[3 = byte +0xc];
  tagMSG local_b8;
  int local_9c;
  undefined1 local_98 [132];
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094820b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_14 = 0;
  if (*(int *)(param_1 + 0x48) != 0) {
    if (*(int *)(param_1 + 0x64 [dec 100]) != 0) {
      FUN_0040de50(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    }
    while (*(int *)(*(int *)(param_1 + 0x48) + 8) != *(int *)(*(int *)(param_1 + 0x48) + 0xc)) {
      FUN_0072fa80(local_c4,&local_9c,5);
      local_8 = 0;
      for (local_c8 = 0; local_c8 < local_9c; local_c8 = local_c8 + 1) {
        iVar1 = *(int *)(*(int *)(local_c8 * 0x10 + local_c4[0] + 0xc) + 0x10);
        if ((*(int *)(local_c8 * 0x10 + local_c4[0]) != 0) &&
           ((((iVar1 == 0x10 || (iVar1 == 0x11)) || (iVar1 == 0x4d)) || (iVar1 == 0x4e)))) {
          *(undefined4 *)(param_1 + 0x14) = 1;
          FUN_0072f9f0();
        }
      }
      _BinkDoFrame_4 [GHIDRA-AUTO-LABEL: unreliable](*(undefined4 *)(param_1 + 0x48));
      iVar1 = _BinkBufferLock_4 [GHIDRA-AUTO-LABEL: unreliable](*(undefined4 *)(param_1 + 0x4c));
      if (iVar1 != 0) {
        _BinkCopyToBuffer_28 [GHIDRA-AUTO-LABEL: unreliable]
                  (*(undefined4 *)(param_1 + 0x48),*(undefined4 *)(*(int *)(param_1 + 0x4c) + 0x14),
                   *(undefined4 *)(*(int *)(param_1 + 0x4c) + 0x18),
                   *(undefined4 *)(*(int *)(param_1 + 0x4c) + 4),0,0,
                   *(undefined4 *)(*(int *)(param_1 + 0x4c) + 0x10));
        _BinkBufferUnlock_4 [GHIDRA-AUTO-LABEL: unreliable](*(undefined4 *)(param_1 + 0x4c));
      }
      uVar2 = _BinkGetRects_8 [GHIDRA-AUTO-LABEL: unreliable](*(undefined4 *)(param_1 + 0x48),
                              *(undefined4 *)(*(int *)(param_1 + 0x4c) + 0x10));
      _BinkBufferBlit_12 [GHIDRA-AUTO-LABEL: unreliable](*(undefined4 *)(param_1 + 0x4c),*(int *)(param_1 + 0x48) + 0x34,uVar2);
      _BinkNextFrame_4 [GHIDRA-AUTO-LABEL: unreliable](*(undefined4 *)(param_1 + 0x48));
      do {
        iVar1 = _BinkWait_4 [GHIDRA-AUTO-LABEL: unreliable](*(undefined4 *)(param_1 + 0x48));
        if (iVar1 != 1) break;
        Sleep(1);
      } while (*(int *)(param_1 + 0x64 [dec 100]) == 0);
      if ((*(int *)(param_1 + 0x14) == 1) || (*(int *)(param_1 + 0x64 [dec 100]) != 0)) {
        local_8 = 0xffffffff;
        Free();
        break;
      }
      while ((BVar3 = PeekMessageA(&local_b8,(HWND)0x0,0,0,0), BVar3 != 0 &&
             (BVar3 = GetMessageA(&local_b8,(HWND)0x0,0,0), BVar3 != 0))) {
        TranslateMessage(&local_b8);
        DispatchMessageA(&local_b8);
      }
      local_8 = 0xffffffff;
      Free();
    }
    _BinkGetSummary_8 [GHIDRA-AUTO-LABEL: unreliable](*(undefined4 *)(param_1 + 0x48),local_98);
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x0040d430 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0040d430: 16-byte aligned: YES
  0040d430  55             PUSH EBP
  0040d431  8B EC          MOV EBP,ESP
  0040d433  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0040d430
string literals (0):
direct callees (6):
  0x00733B60  (unnamed)
  0x0040CC70  CExoMoviePlayerInternal::OpenClip (creates "SWMovieWindow" and Bink)
  0x0040D6F0  Movie::PlayFrames
  0x007338D0  CExoString::operator=_2
  0x0040DA20  Movie::EndClip
  0x0040DFE0  CExoArrayList<CExoString>::SetSize(n)
struct offsets touched (top 10):
  +0x14 [dec 20]  x4  via EAX,ECX,EDX
  +0x1C [dec 28]  x4  via ECX,EDX
  +0x9C [dec 156]  x4  via EAX
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EDX
  +0xC [dec 12]  x2  via EAX,EDX
  +0x18 [dec 24]  x2  via EAX,EDX
  +0x20 [dec 32]  x2  via EAX,ECX
  +0x94 [dec 148]  x2  via ECX,EDX
  +0x30 [dec 48]  x1  via ECX
```

## Card B16-2-34

GOG function: CSWGuiTextParams::SetBaseFont  gog 0x009151b0  callconv=(blank)  param_size_bytes=4  gog_size=36  gog_retn=4
Candidate Steam address: 0x00416de0 (size 36, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00416e30:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00915130 CSWGuiTextParams::SetFontRename -> 0x00416d60  gate_class=REVIEW
- GOG-order neighbour after:  0x009151e0 CSWGuiTextParams::ReSetFont -> 0x00416e10  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0061a2d0 CResRef::CopyToString_2 -> Steam ?  in candidate callees: n/a
  0x00915130 CSWGuiTextParams::SetFontRename -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 6): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x4*,0x3c*} | Steam {0x4*,0x3c*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00416de0: 16-byte aligned: YES
  00416de0  55             PUSH EBP
  00416de1  8B EC          MOV EBP,ESP
  00416de3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00416de0
string literals (0):
direct callees (2):
  0x00710980  (unnamed)
  0x00416D60  CSWGuiTextParams::SetFontRename
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00416de0 FUN_00416de0 size=36 ret=4 callers=6 callees=2

void __fastcall FUN_00416de0(int param_1)

{
  FUN_00710980(param_1 + 0x3c);
  FUN_00416d60();
  return;
}
```

### decoy: 0x00416e30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00416e30: 16-byte aligned: YES
  00416e30  55             PUSH EBP
  00416e31  8B EC          MOV EBP,ESP
  00416e33  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00416e30
string literals (0):
direct callees (3):
  0x007337C0  CExoString::operator=
  0x00734010  CExoString::CStr
  0x00417800  (unnamed)
struct offsets touched (top 4):
  +0x50 [dec 80]  x5  via EAX,EDX
  +0x14 [dec 20]  x3  via EAX,ECX
  +0x8 [dec 8]  x1  via ECX
  +0x18 [dec 24]  x1  via EAX
```

## Card B16-2-35

GOG function: CSWGlobalVariableTable::Save  gog 0x00653500  callconv=(blank)  param_size_bytes=4  gog_size=204  gog_retn=4
Candidate Steam address: 0x006564c0 (size 377, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.06 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00656640:STR1/CALLEE2/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00653380 CSWGlobalVariableTable::ReadTable -> 0x006564c0  gate_class=AUTO
- GOG-order neighbour after:  0x006535d0 CSWGlobalVariableTable::Load -> 0x00656780  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: no
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
  0x00605f00 CExoString::operator+ -> Steam 0x00733df0  in candidate callees: no
  0x00651ed0 CSWGlobalVariableTable::WriteTable -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x007b7980 CServerExoAppInternal::StallEventSaveGame -> Steam 0x00534600  in candidate callers: no
  0x007be170 CServerExoAppInternal::DoPCAutosave -> Steam 0x0053af20  in candidate callers: no
GOG strings (1): 'GLOBALVARS'
disp sets (* = in both, 0 common): GOG {} | Steam {0x94}
imm sets (* = in both, 1 common): GOG {0x2,0x4*,0x5,0x6,0x2c} | Steam {0x4*,0x60,0xa0,0xbe76}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x006564c0: 16-byte aligned: YES
  006564c0  55             PUSH EBP
  006564c1  8B EC          MOV EBP,ESP
  006564c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x006564c0
string literals (1):
  0x00998684  at 006565ad  'CatBoolean'
direct callees (9):
  0x00919723  operator new
  0x00710810  (unnamed)
  0x00716FD0  CResGFF::Constructor_2
  0x008339D0  (unnamed)
  0x007189D0  CResGFF::GetTopLevelStruct
  0x00718760  CResGFF::GetList
  0x00655B00  CSWGlobalVariableTable::ReadTableWithCat (adds unknown names on load)
  0x00655690  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 1):
  +0x94 [dec 148]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006564c0 FUN_006564c0 size=377 ret=4 callers=1 callees=9

void FUN_006564c0(undefined4 param_1)

{
  undefined4 *local_68;
  undefined1 local_4c [16];
  int local_3c;
  undefined4 *local_38;
  undefined4 *local_34;
  int local_30;
  undefined1 local_2c [4];
  undefined1 local_28 [20];
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095904b;
  local_10 = ExceptionList;
  local_14 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_3c = FUN_00919723(0xa0,local_14);
  local_8 = 0;
  if (local_3c == 0) {
    local_68 = (undefined4 *)0x0;
  }
  else {
    FUN_00710810(param_1);
    local_68 = (undefined4 *)FUN_00716fd0(0,&DAT_00998690,local_4c);
  }
  local_38 = local_68;
  local_8 = 0xffffffff;
  local_34 = local_68;
  if ((local_68 == (undefined4 *)0x0) || (local_68[0x25 = byte +0x94] == 0)) {
    FUN_008339d0(0xbe76);
    if ((local_34 != (undefined4 *)0x0) && (local_34 != (undefined4 *)0x0)) {
      (**(code **)*local_34)(1);
    }
  }
  else {
    FUN_007189d0(local_2c);
    local_30 = FUN_00718760(local_28,local_2c,"CatBoolean");
    if (local_30 == 0) {
      FUN_00655690(local_34,local_2c);
    }
    else {
      FUN_00655b00(local_34,local_2c);
    }
    if (local_34 != (undefined4 *)0x0) {
      (**(code **)*local_34)(1);
    }
  }
  ExceptionList = local_10;
  __security_check_cookie [GHIDRA-AUTO-LABEL: unreliable](local_14 ^ (uint)&stack0xfffffffc);
  return;
}
```

### decoy: 0x00656640 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00656640: 16-byte aligned: YES
  00656640  55             PUSH EBP
  00656641  8B EC          MOV EBP,ESP
  00656643  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00656640
string literals (1):
  0x0099874C  at 00656668  'GLOBALVARS'
direct callees (4):
  0x00733570  CExoString::CExoString_char
  0x00733DF0  CExoString::operator+
  0x00733780  CExoString::~CExoString
  0x00655010  (unnamed)
struct offsets touched (top 0):
```

## Card B16-2-36

GOG function: CExoMoviePlayerInternal::Destructor  gog 0x0091fbe0  callconv=(blank)  param_size_bytes=0  gog_size=291  gog_retn=0
Candidate Steam address: 0x0040c9f0 (size 291, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: rotation_in:0x0091fe60:0.71>0.51
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0040c880:STR0/CALLEE0/SIG0.433/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0091fa70 CExoMoviePlayerInternal::Constructor -> 0x0040c880  gate_class=REVIEW
- GOG-order neighbour after:  0x0091fd10 CExoMoviePlayerInternal::ShutDown -> 0x0040cb20  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0044dda0 CExoArrayList_CExoString_::Destructor -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x0060b670 CExoArrayList_T_::Destructor -> Steam ?  in candidate callees: n/a
  0x00610cf0 CExoCriticalSection::Destructor -> Steam ?  in candidate callees: n/a
  0x0091fd10 CExoMoviePlayerInternal::ShutDown -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x38*,0x44*} | Steam {0x4*,0x38*,0x44*}
imm sets (* = in both, 10 common): GOG {0x2*,0x3*,0x4*,0x8*,0x14*,0x28*,0x40*,0x68*,0x90*,0x103*} | Steam {0x2*,0x3*,0x4*,0x8*,0x14*,0x28*,0x40*,0x68*,0x90*,0x103*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0040c9f0: 16-byte aligned: YES
  0040c9f0  55             PUSH EBP
  0040c9f1  8B EC          MOV EBP,ESP
  0040c9f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0040c9f0
string literals (0):
direct callees (5):
  0x0040CB20  (unnamed)
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x00733780  CExoString::~CExoString
  0x00739C90  (unnamed)
  0x0040DF30  (unnamed)
struct offsets touched (top 3):
  +0x44 [dec 68]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x38 [dec 56]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0040c9f0 FUN_0040c9f0 size=291 ret=0 callers=1 callees=8

void __fastcall FUN_0040c9f0(undefined4 *param_1)

{
  DWORD local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_0094819a;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_8 = 4;
  *param_1 = 1;
  SetEvent((HANDLE)param_1[1 = byte +0x4]);
  do {
    GetExitCodeThread((HANDLE)param_1[0xe = byte +0x38],&local_14);
  } while (local_14 == 0x103);
  CloseHandle((HANDLE)param_1[0xe = byte +0x38]);
  CloseHandle((HANDLE)param_1[1 = byte +0x4]);
  if (param_1[0x11 = byte +0x44] != 0) {
    if ((undefined4 *)param_1[0x11 = byte +0x44] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0x11 = byte +0x44])(1);
    }
    param_1[0x11 = byte +0x44] = 0;
  }
  FUN_0040cb20();
  local_8._0_1_ = 3;
  Free();
  local_8._0_1_ = 2;
  FUN_00733780();
  local_8._0_1_ = 1;
  FUN_00739c90();
  local_8 = (uint)local_8._1_3_ << 8;
  FUN_00733780();
  local_8 = 0xffffffff;
  FUN_0040df30();
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x0040c880 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0040c880: 16-byte aligned: YES
  0040c880  55             PUSH EBP
  0040c881  8B EC          MOV EBP,ESP
  0040c883  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0040c880
string literals (0):
direct callees (5):
  0x0040DEF0  (unnamed)
  0x00733540  CExoString::Constructor
  0x00739C10  (unnamed)
  0x0084A640  (unnamed)
  0x007338D0  CExoString::operator=_2
struct offsets touched (top 10):
  +0x4 [dec 4]  x1  via EDX
  +0x14 [dec 20]  x1  via ECX
  +0x18 [dec 24]  x1  via EDX
  +0x1C [dec 28]  x1  via EAX
  +0x20 [dec 32]  x1  via ECX
  +0x24 [dec 36]  x1  via EDX
  +0x30 [dec 48]  x1  via EAX
  +0x34 [dec 52]  x1  via EAX
  +0x38 [dec 56]  x1  via EDX
  +0x44 [dec 68]  x1  via EAX
```

## Card B16-2-37

GOG function: CExoMoviePlayerInternal::ShutDown  gog 0x0091fd10  callconv=(blank)  param_size_bytes=0  gog_size=220  gog_retn=0
Candidate Steam address: 0x0040cb20 (size 220, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0040c880:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0091fbe0 CExoMoviePlayerInternal::Destructor -> 0x0040c9f0  gate_class=REVIEW
- GOG-order neighbour after:  0x0091fdf0 CExoMoviePlayerInternal::RequestRestart -> 0x0040cc00  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x24*,0x48*,0x4c*,0x50*,0x64*} | Steam {0x24*,0x48*,0x4c*,0x50*,0x64*}
imm sets (* = in both, 1 common): GOG {0x84*} | Steam {0x84*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0040cb20: 16-byte aligned: YES
  0040cb20  55             PUSH EBP
  0040cb21  8B EC          MOV EBP,ESP
  0040cb23  81 EC 84 00 00 00 SUB ESP,0x84
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0040cb20
string literals (0):
direct callees (0):
struct offsets touched (top 5):
  +0x48 [dec 72]  x5  via EAX,ECX,EDX
  +0x4C [dec 76]  x3  via EAX,EDX
  +0x24 [dec 36]  x2  via EAX
  +0x50 [dec 80]  x2  via EAX,EDX
  +0x64 [dec 100]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0040cb20 FUN_0040cb20 size=220 ret=0 callers=3 callees=5

void __fastcall FUN_0040cb20(int param_1)

{
  undefined1 local_84 [128];
  
  *(undefined4 *)(param_1 + 0x24) = 1;
  if (*(int *)(param_1 + 0x48) != 0) {
    _BinkGetSummary_8 [GHIDRA-AUTO-LABEL: unreliable](*(undefined4 *)(param_1 + 0x48),local_84);
    _BinkPause_8 [GHIDRA-AUTO-LABEL: unreliable](*(undefined4 *)(param_1 + 0x48),1);
    if (*(int *)(param_1 + 0x64 [dec 100]) == 0) {
      _BinkClose_4 [GHIDRA-AUTO-LABEL: unreliable](*(undefined4 *)(param_1 + 0x48));
      *(undefined4 *)(param_1 + 0x48) = 0;
    }
    if (*(int *)(param_1 + 0x4c) != 0) {
      _BinkBufferClose_4 [GHIDRA-AUTO-LABEL: unreliable](*(undefined4 *)(param_1 + 0x4c));
      *(undefined4 *)(param_1 + 0x4c) = 0;
    }
    DestroyWindow(*(HWND *)(param_1 + 0x50));
    *(undefined4 *)(param_1 + 0x50) = 0;
    DAT_00a1b9e8 = 0;
  }
  *(undefined4 *)(param_1 + 0x24) = 0;
  return;
}
```

### decoy: 0x0040c880 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0040c880: 16-byte aligned: YES
  0040c880  55             PUSH EBP
  0040c881  8B EC          MOV EBP,ESP
  0040c883  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0040c880
string literals (0):
direct callees (5):
  0x0040DEF0  (unnamed)
  0x00733540  CExoString::Constructor
  0x00739C10  (unnamed)
  0x0084A640  (unnamed)
  0x007338D0  CExoString::operator=_2
struct offsets touched (top 10):
  +0x4 [dec 4]  x1  via EDX
  +0x14 [dec 20]  x1  via ECX
  +0x18 [dec 24]  x1  via EDX
  +0x1C [dec 28]  x1  via EAX
  +0x20 [dec 32]  x1  via ECX
  +0x24 [dec 36]  x1  via EDX
  +0x30 [dec 48]  x1  via EAX
  +0x34 [dec 52]  x1  via EAX
  +0x38 [dec 56]  x1  via EDX
  +0x44 [dec 68]  x1  via EAX
```

## Card B16-2-38

GOG function: CSWGlobalVariableTable::ReadTableWithoutCatalogue  gog 0x00652550  callconv=(blank)  param_size_bytes=8  gog_size=1110  gog_retn=8
Candidate Steam address: 0x00655690 (size 1110, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.73) | G8 mac_rotation: pass
- anchors: A-STR 5 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00655b00:STR5/CALLEE1/SIG0.76/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00651ed0 CSWGlobalVariableTable::WriteTable -> 0x00655010  gate_class=AUTO
- GOG-order neighbour after:  0x006529c0 CSWGlobalVariableTable::ReadTableWithCatalogue -> 0x00655b00  gate_class=REJECT

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (14), mapped to Steam through trusted rows only:
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x006248c0 CResGFF::GetList -> Steam 0x00718760  in candidate callees: yes
  0x00624970 CResGFF::GetListCount -> Steam 0x00718810  in candidate callees: yes
  0x006249f0 CResGFF::GetListElement -> Steam 0x00718890  in candidate callees: yes
  0x006250f0 CResGFF::ReadFieldCExoString -> Steam 0x00718f90  in candidate callees: yes
  0x006257a0 CResGFF::ReadFieldVOID -> Steam 0x00719640  in candidate callees: yes
  ... 6 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (5): 'String'; 'ValString'; 'ValLocation'; 'ValNumber' ...
disp sets (* = in both, 9 common): GOG {0x4*,0x8*,0xc*,0x10*,0x14*,0x16*,0xde29*,0xe214*,0xeb74*} | Steam {0x4*,0x8*,0xc*,0x10*,0x14*,0x16*,0xde29*,0xe214*,0xeb74*}
imm sets (* = in both, 15 common): GOG {0x2*,0x3*,0x5*,0x8*,0xc*,0xe*,0x18*,0x64*,0x71*,0x3e8*,0x93d*,0x960*,0xe4c*,0x3fff*,...(+1)} | Steam {0x2*,0x3*,0x5*,0x8*,0xc*,0xe*,0x18*,0x64*,0x71*,0x3e8*,0x93d*,0x960*,0xe4c*,0x3fff*,...(+1)}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00655690: 16-byte aligned: YES
  00655690  55             PUSH EBP
  00655691  8B EC          MOV EBP,ESP
  00655693  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00655690
string literals (5):
  0x00998654  at 00655736  'ValBoolean'
  0x00998648  at 00655758  'ValNumber'
  0x0099863C  at 0065577d  'ValLocation'
  0x00998630  at 006557a4  'ValString'
  0x00998628  at 00655853  'String'
direct callees (14):
  0x0091D67E  (unnamed)
  0x0091C630  (unnamed)
  0x00719640  CResGFF::ReadFieldVOID
  0x00718760  CResGFF::GetList
  0x00718810  CResGFF::GetListCount
  0x00718890  CResGFF::GetListElement
  0x00733570  CExoString::CExoString_char
  0x00718F90  CResGFF::ReadFieldCExoString
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x00654670  (unnamed)
  0x006546C0  (unnamed)
  0x00919BCC  `eh vector destructor iterator` (CRT)
  0x009196EE  __security_check_cookie
struct offsets touched (top 5):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0xC [dec 12]  x2  via ECX,EDX
  +0x10 [dec 16]  x2  via ECX,EDX
  +0x14 [dec 20]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00655690 FUN_00655690 size=1110 ret=8 callers=1 callees=14

void __thiscall FUN_00655690(int param_1,undefined4 param_2,undefined4 param_3)

{
  undefined4 *puVar1;
  int iVar2;
  uint uVar3;
  undefined4 uVar4;
  int local_e50;
  undefined1 local_e48 [8];
  undefined1 local_e40 [8];
  undefined1 local_e38 [120];
  undefined1 local_dc0 [4];
  int local_dbc;
  undefined4 local_db8[601 = byte +0x964];
  int local_454;
  undefined1 local_450 [40];
  undefined1 local_428 [1004];
  undefined1 local_3c [20];
  uint local_28;
  undefined1 local_24 [4];
  int local_20;
  undefined1 local_19;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00958f12;
  local_10 = ExceptionList;
  local_28 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  _eh_vector_constructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable](local_db8,0x18,100,FUN_0053e060,FUN_00623320);
  local_8 = 0;
  _eh_vector_constructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable](local_450,8,5,FUN_00733540,FUN_00733780);
  local_8 = CONCAT31(local_8._1_3_,1);
  local_19 = 0;
  _memset [GHIDRA-AUTO-LABEL: unreliable](local_e38,0,0x71);
  _memset [GHIDRA-AUTO-LABEL: unreliable](local_428,0,1000);
  FUN_00719640(param_3,local_e38,0x71,"ValBoolean",local_24,&local_19);
  FUN_00719640(param_3,local_428,1000,"ValNumber",local_24,&local_19);
  FUN_00719640(param_3,local_db8,0x960,"ValLocation",local_24,&local_19);
  local_454 = 0;
  iVar2 = FUN_00718760(local_3c,param_3,"ValString");
  if (iVar2 != 0) {
    uVar3 = FUN_00718810(local_3c);
    if (uVar3 < 6) {
      local_e50 = FUN_00718810(local_3c);
    }
    else {
      local_e50 = 5;
    }
    local_454 = local_e50;
    for (local_20 = 0; local_20 < local_454; local_20 = local_20 + 1) {
      FUN_00718890(local_dc0,local_3c,local_20);
      FUN_00733570(&DAT_00986925);
      local_8._0_1_ = 2;
      uVar4 = FUN_00718f90(local_e48,local_dc0,"String",local_24,local_e40);
      local_8._0_1_ = 3;
      FUN_007337c0(uVar4);
      local_8._0_1_ = 2;
      FUN_00733780();
      local_8 = CONCAT31(local_8._1_3_,1);
      FUN_00733780();
    }
  }
  local_18 = 0;
  local_dbc = 0;
  local_14 = 0;
  for (local_20 = 0; local_20 < 0x93d; local_20 = local_20 + 1) {
    if (*(char *)(param_1 + local_20 * 0x18) != '\0') {
      switch(*(ushort *)(param_1 + 0x16 + local_20 * 0x18) >> 0xe) {
      case 0:
        uVar4 = FUN_00654670(local_e38,local_18);
        FUN_006546c0(param_1 + 0xddb8,*(ushort *)(param_1 + 0x16 + local_20 * 0x18) & 0x3fff,uVar4);
        local_18 = local_18 + 1;
        break;
      case 1:
/* ... 31 more lines truncated by --max-lines */
```

### decoy: 0x00655b00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00655b00: 16-byte aligned: YES
  00655b00  55             PUSH EBP
  00655b01  8B EC          MOV EBP,ESP
  00655b03  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00655b00
string literals (13):
  0x00998654  at 00655bc7  'ValBoolean'
  0x00998684  at 00655beb  'CatBoolean'
  0x00998720  at 00655d37  "ReadTableWithCat(): BOOLEAN '%s' won't fit!"
  0x00998648  at 00655dbf  'ValNumber'
  0x00998678  at 00655de6  'CatNumber'
  0x009986F4  at 00655f32  "ReadTableWithCat(): NUMBER '%s' won't fit!"
  0x0099863C  at 00655f95  'ValLocation'
  0x0099866C  at 00655fbc  'CatLocation'
  0x009986C4  at 00656108  "ReadTableWithCat(): LOCATION '%s' won't fit!"
  0x00998630  at 0065618b  'ValString'
  0x00998628  at 0065623a  'String'
  0x00998660  at 006562a4  'CatString'
direct callees (24):
  0x0091D67E  (unnamed)
  0x00733540  CExoString::Constructor
  0x00653DD0  (unnamed)
  0x0091C630  (unnamed)
  0x00719640  CResGFF::ReadFieldVOID
  0x00718760  CResGFF::GetList
  0x00718810  CResGFF::GetListCount
  0x00718890  CResGFF::GetListElement
  0x00733570  CExoString::CExoString_char
  0x00718F90  CResGFF::ReadFieldCExoString
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x00733B60  (unnamed)
  0x006545B0  GlobalVars Find thiscall(table, CExoString* name) RET 4 (AddBoolean 0x654430 has no dup check)
  0x00654430  CSWGlobalVariableTable::AddBoolean(name)
  0x00734010  CExoString::CStr
  0x00734270  CExoString::Format
  0x00654670  (unnamed)
  0x006546C0  (unnamed)
  0x00654490  CSWGlobalVariableTable::AddNumber(name)
struct offsets touched (top 5):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0xC [dec 12]  x2  via ECX,EDX
  +0x10 [dec 16]  x2  via ECX,EDX
  +0x14 [dec 20]  x2  via ECX,EDX
```

## Card B16-2-39

GOG function: CSWGuiInGameCharacter::OnPanelRemoved  gog 0x005991e0  callconv=(blank)  param_size_bytes=0  gog_size=32  gog_retn=0
Candidate Steam address: 0x0084e6f0 (size 4154, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CSWGuiInGameCharacter::SetStats | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.01 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0084e6c0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00599070 CSWGuiInGameCharacter::SetModText -> 0x0084f770  gate_class=REVIEW
- GOG-order neighbour after:  0x00599200 CSWGuiInGameCharacter::HandleInputEvent -> 0x0084f900  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0090d940 CSWGuiPanel::OnPanelRemoved -> Steam 0x0040f4c0  in candidate callees: no
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4b70*} | Steam {0x4,0x32,0x48,0x4a,0x4c,0x68,0x80,0x88,0x8c,0x94,0xec,0xed,0xee,0xef,...(+22)}
imm sets (* = in both, 0 common): GOG {} | Steam {0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xf,0x10,...(+52)}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0084e6f0: 16-byte aligned: YES
  0084e6f0  55             PUSH EBP
  0084e6f1  8B EC          MOV EBP,ESP
  0084e6f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0084e6f0
string literals (2):
  0x009A3EEC  at 0084e7f7  '%d/%d'
  0x009A3EEC  at 0084e940  '%d/%d'
direct callees (35):
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5DA0  Party member entry thiscall(party, int idx) RET 4 (0x7E5D80 = idx 0)
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x00844E40  CSWSCreatureStats::GetClassByPosition
  0x00733540  CExoString::Constructor
  0x00848CF0  (unnamed)
  0x00734270  CExoString::Format
  0x00416E30  CSWGuiTextParams::SetText
  0x00850090  (unnamed)
  0x00844A70  (unnamed)
  0x00733700  CExoString::Constructor_5
  0x00733780  CExoString::~CExoString
  0x00417010  (unnamed)
  0x006F2A30  IsForceUserClass(classId) {3,4,5,11..16}
  0x006B5990  CSWSCreatureStats::GetSTRStat
  0x0084F770  (unnamed)
  0x006B5BA0  CSWSCreatureStats::GetWISStat
  0x006B5C50  CSWSCreatureStats::GetCHAStat
  0x006B5AF0  CSWSCreatureStats::GetINTStat
  0x006B5A40  CSWSCreatureStats::GetCONStat
struct offsets touched (top 10):
  +0x48 [dec 72]  x26  via EAX,ECX,EDX
  +0x4B6C [dec 19308]  x11  via EAX,ECX,EDX
  +0x4B74 [dec 19316]  x3  via EAX,ECX,EDX
  +0x80 [dec 128]  x2  via EAX,EDX
  +0x12A [dec 298]  x2  via EAX,EDX
  +0x12C [dec 300]  x2  via EAX,ECX
  +0x224 [dec 548]  x2  via EAX,EDX
  +0x310 [dec 784]  x2  via EAX
  +0x4B70 [dec 19312]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0084e6f0 FUN_0084e6f0 size=4154 ret=0 callers=3 callees=35

/* WARNING: Removing unreachable block (ram,0x0084e8d4) */

void __fastcall FUN_0084e6f0(int param_1)

{
  char cVar1;
  char cVar2;
  short sVar3;
  short sVar4;
  undefined4 *puVar5;
  int iVar6;
  undefined4 *puVar7;
  undefined4 uVar8;
  undefined4 local_120;
  undefined4 local_11c;
  undefined4 local_118;
  undefined4 local_114;
  undefined4 local_110;
  undefined4 local_10c;
  undefined1 local_108 [16];
  undefined1 local_f8 [8];
  undefined1 local_f0 [8];
  undefined1 local_e8 [8];
  undefined1 local_e0 [8];
  undefined1 local_d8 [8];
  undefined1 local_d0 [8];
  undefined1 local_c8 [8];
  undefined1 local_c0 [8];
  undefined1 local_b8 [8];
  undefined1 local_b0 [8];
  undefined1 local_a8 [8];
  undefined1 local_a0 [8];
  int local_98;
  int local_94;
  undefined4 local_90[4 = byte +0x10];
  undefined4 local_80;
  uint local_54;
  int local_50;
  int local_4c;
  int local_48;
  byte local_41;
  uint local_40;
  undefined4 local_3c;
  int local_38;
  int local_34;
  undefined4 local_30;
  int local_2c;
  uint local_28;
  undefined1 local_24 [8];
  uint local_1c;
  int *local_18;
  char local_11;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00971d8c;
  local_10 = ExceptionList;
  local_54 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_30 = *(undefined4 *)(DAT_00a1b4a4 + 4);
  puVar5 = (undefined4 *)FUN_0073fb90(local_54);
  *(undefined4 *)(param_1 + 0x4b68) = *puVar5;
  uVar8 = 0;
  FUN_0073fb90(0);
  local_18 = (int *)FUN_007e5da0(uVar8);
  local_38 = local_18[0xc4 = byte +0x310];
  local_48 = FUN_0077d800();
  local_2c = *(int *)(local_48 + 0x1198);
  local_11 = *(char *)(local_38 + 0x32) + -1;
  local_41 = FUN_00844e40(local_11);
  FUN_00733540();
  local_8 = 0;
  sVar4 = *(short *)(local_38 + 0x4c);
  sVar3 = FUN_00848cf0();
  FUN_00734270(local_24,"%d/%d",(int)sVar4,(int)sVar3);
  FUN_00416e30(local_24);
  FUN_00850090(param_1 + 0x1110,(int)*(short *)(local_38 + 0x4a));
/* ... 240 more lines truncated by --max-lines */
```

### decoy: 0x0084e6c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0084e6c0: 16-byte aligned: YES
  0084e6c0  55             PUSH EBP
  0084e6c1  8B EC          MOV EBP,ESP
  0084e6c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0084e6c0
string literals (0):
direct callees (3):
  0x0084E6F0  CSWGuiInGameCharacter::SetStats (CanLevelUp at 0x84F221 -> button visibility)
  0x00623320  CSWSEncounter::AIUpdate
  0x0040F3C0  CSWGuiPanel::OnPanelAdded
struct offsets touched (top 0):
```

## Card B16-2-40

GOG function: CSWGuiTextParams::SetFontRename  gog 0x00915130  callconv=__thiscall  param_size_bytes=0  gog_size=126  gog_retn=0
Candidate Steam address: 0x00416d60 (size 126, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00416de0:STR0/CALLEE0/SIG0.1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00915090 CSWGuiTextParams::SetFont -> 0x00416cc0  gate_class=AUTO
- GOG-order neighbour after:  0x009151b0 CSWGuiTextParams::SetBaseFont -> 0x00416de0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x0061a190 CResRef::Constructor_4 -> Steam ?  in candidate callees: n/a
  0x0061a2d0 CResRef::CopyToString_2 -> Steam ?  in candidate callees: n/a
  0x00912160 CSWGuiManager::GetUpdatedFontName -> Steam ?  in candidate callees: n/a
  0x00915090 CSWGuiTextParams::SetFont -> Steam ?  in candidate callees: n/a
  0x00921312 Global::@__security_check_cookie@4 -> Steam ?  in candidate callees: n/a
  0x00924c20 Global::strcpy -> Steam ?  in candidate callees: n/a
  0x0093e852 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0x8*,0xc*,0x3c*,0x40*} | Steam {0x8*,0xc*,0x3c*,0x40*}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00416d60: 16-byte aligned: YES
  00416d60  55             PUSH EBP
  00416d61  8B EC          MOV EBP,ESP
  00416d63  83 EC 40       SUB ESP,0x40
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00416d60
string literals (0):
direct callees (7):
  0x0091CFF0  (unnamed)
  0x00413CE0  CSWGuiManager::ResolveFontName (appends 'b')
  0x00710980  (unnamed)
  0x009370C7  _stricmp
  0x00710840  (unnamed)
  0x00416CC0  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00416d60 FUN_00416d60 size=126 ret=0 callers=3 callees=7

void __fastcall FUN_00416d60(int param_1)

{
  int iVar1;
  undefined1 local_40 [16];
  char local_30 [20];
  char local_1c [20];
  uint local_8;
  
  local_8 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  FUN_0091cff0(local_30,param_1 + 0x3c);
  FUN_00413ce0(local_30);
  FUN_00710980(local_1c);
  iVar1 = __stricmp [GHIDRA-AUTO-LABEL: unreliable](local_30,local_1c);
  if (iVar1 != 0) {
    FUN_00710840(local_30);
    FUN_00416cc0(local_40);
  }
  __security_check_cookie [GHIDRA-AUTO-LABEL: unreliable](local_8 ^ (uint)&stack0xfffffffc);
  return;
}
```

### decoy: 0x00416de0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00416de0: 16-byte aligned: YES
  00416de0  55             PUSH EBP
  00416de1  8B EC          MOV EBP,ESP
  00416de3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00416de0
string literals (0):
direct callees (2):
  0x00710980  (unnamed)
  0x00416D60  CSWGuiTextParams::SetFontRename
struct offsets touched (top 0):
```

## Card B16-2-41

GOG function: CServerAIMaster::ClearEventQueue  gog 0x0064da00  callconv=(blank)  param_size_bytes=0  gog_size=91  gog_retn=0
Candidate Steam address: 0x0051d790 (size 2113, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CServerAIMaster::UpdateState | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.03 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051dfe0:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064d1b0 CServerAIMaster::UpdateState -> 0x0051d790  gate_class=AUTO
- GOG-order neighbour after:  0x0064da60 CServerAIMaster::AddObject -> 0x0051e040  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00477c30 unnamed -> Steam ?  in candidate callees: n/a
  0x00650700 CServerAIMaster::DeleteEventData -> Steam ?  in candidate callees: n/a
  0x007c0300 CExoLinkedList_CAurObject_::RemoveHead -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x007b4960 CServerExoAppInternal::UnloadModule -> Steam 0x005315e0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x10,0x14} | Steam {0x4,0x8,0xc,0x20,0x28,0x30,0x68,0x6c,0x70,0x74,0x78,0x7c,0xbc,0xd8,...(+6)}
imm sets (* = in both, 1 common): GOG {0x4*,0xc,0x54} | Steam {0x2,0x4*,0x5,0x6,0x3c,0x3d,0x46,0x60,0x64,0x94,0xa0,0xa3,0xf0,0x100,...(+2)}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051d790: 16-byte aligned: YES
  0051d790  55             PUSH EBP
  0051d791  8B EC          MOV EBP,ESP
  0051d793  81 EC F0 00 00 00 SUB ESP,0xf0
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051d790
string literals (0):
direct callees (21):
  0x0051AD20  CWorldTimer::GetWorldTime
  0x0091FC30  (unnamed)
  0x0091D220  (unnamed)
  0x00736220  CExoTimers::GetHighResolutionTimer
  0x0051D270  CServerAIList::GetObjectAtPosition
  0x00521060  (unnamed)
  0x005C3F70  CSWSCreature::WalkUpdateLocation_QuickWalk
  0x0051ED10  CServerAIMaster::EventPending(day,ms)
  0x0051ED80  CServerAIMaster::GetPendingEvent
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005221B0  CSWSArea::EventHandler
  0x00557440  CSWSModule::EventHandler
  0x00520CE0  Events::FreePayload(type,ptr)
  0x0051D1C0  CServerAIList::GetNextObject
  0x007338D0  CExoString::operator=_2
  0x00549F70  (unnamed)
  0x0054A840  CSWSObject::GetClientObject
  0x0054A480  CSWSObject::IsDirtyFlagSet
  0x0054A4D0  CSWSObject::ClearDirtyFlag
  0x0052F4C0  CServerExoAppInternal::GetModule
struct offsets touched (top 10):
  +0x30 [dec 48]  x7  via EDX
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0xC [dec 12]  x4  via EDX
  +0x11B0 [dec 4528]  x4  via ECX
  +0x8 [dec 8]  x3  via EAX,ECX
  +0x70 [dec 112]  x2  via EDX
  +0x78 [dec 120]  x2  via EAX,ECX
  +0xD8 [dec 216]  x2  via EDX
  +0x220 [dec 544]  x2  via EDX
  +0x20 [dec 32]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0051d790 FUN_0051d790 size=2113 ret=0 callers=1 callees=21

/* WARNING: Removing unreachable block (ram,0x0051dbb4) */
/* WARNING: Removing unreachable block (ram,0x0051dd24) */
/* WARNING: Removing unreachable block (ram,0x0051dd58) */

void __fastcall FUN_0051d790(int *param_1)

{
  char cVar1;
  int iVar2;
  int *piVar3;
  int iVar4;
  int *piVar5;
  uint uVar6;
  undefined8 uVar7;
  ulonglong uVar8;
  longlong lVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  undefined4 uVar12;
  undefined4 uVar13;
  undefined4 uVar14;
  int local_78;
  undefined4 local_70;
  int *local_6c;
  undefined4 local_68;
  int *local_64;
  undefined4 local_60;
  undefined4 local_5c;
  undefined4 local_58;
  undefined4 local_54;
  int *local_50;
  int *local_4c;
  int local_48;
  int local_44;
  int *local_40;
  int local_3c;
  int local_38;
  undefined8 local_34;
  int *local_2c;
  undefined4 local_28;
  undefined8 local_24;
  uint local_1c;
  int local_18;
  undefined8 local_14;
  undefined4 local_8;
  
  local_1c = 10000;
  local_18 = 0;
  local_38 = 4;
  FUN_0051ad20(&local_8,&local_28);
  do {
    if (local_38 < 0) {
      for (local_38 = 4; -1 < local_38; local_38 = local_38 + -1) {
        for (local_78 = 0; local_78 < param_1[local_38 * 4 + 2]; local_78 = local_78 + 1) {
          iVar2 = FUN_0051d270(local_78);
          if ((iVar2 != 0) && (*(int *)(iVar2 + 0x220) != 0)) {
            piVar3 = (int *)FUN_0054a840();
            if (piVar3 != (int *)0x0) {
              iVar4 = (**(code **)(*piVar3 + 0xc))();
              if (iVar4 != 0) {
                iVar4 = FUN_0054a480(0);
                if (iVar4 != 0) {
                  piVar5 = (int *)(**(code **)(*piVar3 + 0xc))();
                  (**(code **)(*piVar5 + 0x134))
                            (*(undefined4 *)(iVar2 + 0xd8),*(undefined4 *)(iVar2 + 0xdc));
                  FUN_0054a4d0(0);
                }
                iVar4 = FUN_0054a480(1);
                if (iVar4 != 0) {
                  piVar5 = (int *)(**(code **)(*piVar3 + 0xc))();
                  (**(code **)(*piVar5 + 0x12c [dec 300] {int*: byte +0x4b0}))
                            (iVar2 + 0x94,*(undefined4 *)(iVar2 + 0xd8),
                             *(undefined4 *)(iVar2 + 0xbc));
                  FUN_0054a4d0(1);
                }
                iVar4 = FUN_0054a480(2);
                if (iVar4 != 0) {
                  piVar3 = (int *)(**(code **)(*piVar3 + 0xc))();
                  (**(code **)(*piVar3 + 0x130))(iVar2 + 0xa0);
/* ... 142 more lines truncated by --max-lines */
```

### decoy: 0x0051dfe0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051dfe0: 16-byte aligned: YES
  0051dfe0  55             PUSH EBP
  0051dfe1  8B EC          MOV EBP,ESP
  0051dfe3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051dfe0
string literals (0):
direct callees (4):
  0x007A1780  (unnamed)
  0x007F2A90  (unnamed)
  0x00520CE0  Events::FreePayload(type,ptr)
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x10 [dec 16]  x1  via EDX
  +0x14 [dec 20]  x1  via EAX
```
