# Review cards B17 chunk 4

Answer each card with one TSV row in `steam_db_batches/B17/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B17-4-01

GOG function: Global::ImageGetSubImage  gog 0x008d8c90  callconv=(blank)  param_size_bytes=8  gog_size=176  gog_retn=0
Candidate Steam address: 0x00483ef0 (size 528, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00484100:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x008d8a80 Global::ImageLowpassBlur  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008d8d40 Global::AurImageApplyLowEndEnvMap  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
  0x00925530 Global::memcpy -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x4*,0xc*,0x20} | Steam {0x3,0x4*,0xc*,0x5c,0xc00}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 0)
```

### prologue
```
prologue for 0x00483ef0: 16-byte aligned: YES
  00483ef0  55             PUSH EBP
  00483ef1  8B EC          MOV EBP,ESP
  00483ef3  83 EC 5C       SUB ESP,0x5c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00483ef0
string literals (0):
direct callees (4):
  0x00919723  operator new
  0x0091C630  (unnamed)
  0x0091DBA0  _memcpy
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00483ef0 FUN_00483ef0 size=528 ret=0 callers=2 callees=4

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00483ef0(void *param_1,int param_2,int param_3,int param_4)

{
  void *_Dst [GHIDRA-AUTO-LABEL: unreliable];
  char local_60;
  int local_48;
  int local_44;
  int local_40;
  int local_3c;
  int local_2c;
  float local_28[9 = byte +0x24];
  
  local_28[0] = _DAT_0098c5a0;
  local_28[1 = byte +0x4] = (float)_DAT_009876fc;
  local_28[2 = byte +0x8] = _DAT_0098c5a0;
  local_28[3 = byte +0xc] = (float)_DAT_009876fc;
  local_28[4 = byte +0x10] = (float)_DAT_0098c014;
  local_28[5 = byte +0x14] = (float)_DAT_009876fc;
  local_28[6 = byte +0x18] = _DAT_0098c5a0;
  local_28[7 = byte +0x1c] = (float)_DAT_009876fc;
  local_28[8 = byte +0x20] = _DAT_0098c5a0;
  _Dst [GHIDRA-AUTO-LABEL: unreliable] = (void *)FUN_00919723(param_2 * param_3 * param_4);
  _memset [GHIDRA-AUTO-LABEL: unreliable](_Dst [GHIDRA-AUTO-LABEL: unreliable],0,param_2 * param_3 * param_4);
  for (local_2c = 0; local_2c < param_3; local_2c = local_2c + 1) {
    for (local_48 = 0; local_48 < param_2; local_48 = local_48 + 1) {
      for (local_3c = 0; local_3c < 3; local_3c = local_3c + 1) {
        for (local_40 = 0; local_40 < 3; local_40 = local_40 + 1) {
          for (local_44 = 0; local_44 < param_4; local_44 = local_44 + 1) {
            local_60 = (char)(int)ROUND((float)*(byte *)((int)param_1 +
                                                        (((local_2c + param_3 + -1 + local_3c) %
                                                         param_3) * param_2 +
                                                        (local_48 + param_2 + -1 + local_40) %
                                                        param_2) * param_4 + local_44) *
                                        local_28[local_3c * 3 + local_40]);
            *(char *)((int)_Dst [GHIDRA-AUTO-LABEL: unreliable] + (local_2c * param_2 + local_48) * param_4 + local_44) =
                 *(char *)((int)_Dst [GHIDRA-AUTO-LABEL: unreliable] + (local_2c * param_2 + local_48) * param_4 + local_44) +
                 local_60;
          }
        }
      }
    }
  }
  _memcpy [GHIDRA-AUTO-LABEL: unreliable](param_1,_Dst [GHIDRA-AUTO-LABEL: unreliable],param_2 * param_3 * param_4);
  FUN_009196fd(_Dst [GHIDRA-AUTO-LABEL: unreliable]);
  return;
}
```

### decoy: 0x00484100 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00484100: 16-byte aligned: YES
  00484100  55             PUSH EBP
  00484101  8B EC          MOV EBP,ESP
  00484103  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00484100
string literals (0):
direct callees (2):
  0x00919723  operator new
  0x0091DBA0  _memcpy
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 528 B, RET 0
- callers 2: 0x00490fd0, 0x00492bf0
- callees: 0x00919723 operator new (CALL), 0x0091c630 (CALL), 0x0091dba0 _memcpy (CALL), 0x009196fd operator delete (CALL)
- twins: none

## Card B17-4-02

GOG function: CSWBehaviorCameraDialog::GetCameraParameters  gog 0x005750f0  callconv=(blank)  param_size_bytes=40  gog_size=8676  gog_retn=40
Candidate Steam address: 0x008c2960 (size 8676, retn 40)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.88) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008c4b50:STR0/CALLEE0/SIG0.561/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00574be0 CSWBehaviorCameraDialog::Control  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005772e0 CSWBehaviorCameraDialog::GetSideUnobstructed  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (12), mapped to Steam through trusted rows only:
  0x0040c990 CClientExoApp::GetGameObject -> Steam 0x0073f4d0  in candidate callees: yes
  0x00472990 CSWCObject::GetAnimationBase -> Steam 0x007ed830  in candidate callees: yes
  0x0080c0a0 Global::float_magnitude_Vector_const_ref -> Steam ?  in candidate callees: n/a
  0x0080c170 Global::normalize -> Steam ?  in candidate callees: n/a
  0x0080d4b0 Global::YawPitchRoll -> Steam ?  in candidate callees: n/a
  0x0080dbe0 Global::Yaw_2 -> Steam ?  in candidate callees: n/a
  0x0080dc60 Global::Pitch_2 -> Steam ?  in candidate callees: n/a
  0x0080f810 Global::equal -> Steam ?  in candidate callees: n/a
  ... 4 more GOG callees
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 16 common): GOG {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x1c*,0x20*,0x24*,0x34*,0x38*,0x5c*,0x60*,0x64*,...(+2)} | Steam {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x1c*,0x20*,0x24*,0x34*,0x38*,0x5c*,0x60*,0x64*,...(+2)}
imm sets (* = in both, 11 common): GOG {0x2*,0x3*,0x4*,0x8*,0xc*,0x10*,0x28*,0x44*,0xfe*,0x6c8*,0xbfffffff*} | Steam {0x2*,0x3*,0x4*,0x8*,0xc*,0x10*,0x28*,0x44*,0xfe*,0x6c8*,0xbfffffff*}

### abi
```
GOG param bytes: 40; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(40/4)*4 = 40 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=40 ; asm RET = 40
Stack Slot Match: TRUE (GOG 40 bytes -> x86 slots 40; ASM RET 40)
```

### prologue
```
prologue for 0x008c2960: 16-byte aligned: YES
  008c2960  55             PUSH EBP
  008c2961  8B EC          MOV EBP,ESP
  008c2963  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x008c2960
string literals (0):
direct callees (12):
  0x00517980  Vec3::Equal
  0x005142F0  (unnamed)
  0x00515620  (unnamed)
  0x005143C0  (unnamed)
  0x00465560  (unnamed)
  0x004491C0  CExoPtrList::Add (doubling)
  0x0073F4D0  CClientExoApp::GetGameObject
  0x007ED830  CSWCObject::GetAnimationBase
  0x00473E80  (unnamed)
  0x00473E50  operator delete[] wrapper (`if (*p) FUN_0091c6aa(*p)`)
  0x00515DD0  (unnamed)
  0x00515D50  (unnamed)
struct offsets touched (top 10):
  +0x8 [dec 8]  x62  via EAX,ECX,EDX
  +0x4 [dec 4]  x48  via EAX,ECX,EDX
  +0xC [dec 12]  x28  via EAX,ECX,EDX
  +0x34 [dec 52]  x5  via EAX,EDX
  +0x1C [dec 28]  x4  via EAX,ECX,EDX
  +0x20 [dec 32]  x4  via ECX,EDX
  +0x10 [dec 16]  x2  via ECX,EDX
  +0x38 [dec 56]  x2  via ECX,EDX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 008c2960 FUN_008c2960 size=8676 ret=40 callers=3 callees=12

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* WARNING: Restarted to delay deadcode elimination for space: stack */

void __thiscall
FUN_008c2960(int param_1,int param_2,int param_3,float param_4,float param_5,float param_6,
            float param_7,float param_8,float param_9,float *param_10,undefined4 *param_11)

{
  float fVar1;
  int iVar2;
  float *pfVar3;
  int *piVar4;
  undefined4 uVar5;
  undefined4 *puVar6;
  float10 fVar7;
  undefined1 local_3dc [16];
  undefined1 local_3cc [12];
  undefined1 local_3c0 [12];
  float local_3b4;
  float local_3b0;
  float local_3ac;
  float local_3a8;
  float local_3a4;
  float local_3a0;
  float local_39c;
  float local_398;
  float local_394;
  undefined1 local_390 [12];
  undefined1 local_384 [12];
  float local_378;
  float local_374;
  float local_370;
  undefined1 local_36c [16];
  float local_35c;
  float local_358;
  float local_354;
  float local_350;
  float local_34c;
  float local_348;
  float local_344;
  float local_340;
  float local_33c;
  undefined1 local_338 [12];
  float local_32c;
  float local_328;
  float local_324;
  float local_320;
  float local_31c;
  float local_318;
  float local_314;
  float local_310;
  float local_30c;
  float local_308;
  float local_304;
  float local_300;
  float local_2fc;
  float local_2f8;
  float local_2f4;
  float local_2f0;
  float local_2ec;
  float local_2e8;
  float local_2e4;
  float local_2e0;
  float local_2dc;
  undefined1 local_2d8 [12];
  float local_2cc;
  float local_2c8;
  float local_2c4;
  undefined1 local_2c0 [16];
  float local_2b0;
  float local_2ac;
  float local_2a8;
  float local_2a4;
  float local_2a0;
  float local_29c;
  float local_298;
  float local_294;
  float local_290;
  float local_28c;
/* ... 580 more lines truncated by --max-lines */
```

### decoy: 0x008c4b50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008c4b50: 16-byte aligned: YES
  008c4b50  55             PUSH EBP
  008c4b51  8B EC          MOV EBP,ESP
  008c4b53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x008c4b50
string literals (0):
direct callees (9):
  0x0044C8A0  Vector::operator+ (Vector3 add)
  0x00465560  (unnamed)
  0x004491C0  CExoPtrList::Add (doubling)
  0x0073F4D0  CClientExoApp::GetGameObject
  0x007ED830  CSWCObject::GetAnimationBase
  0x008C2960  (unnamed)
  0x00465650  (unnamed)
  0x00473E80  (unnamed)
  0x00473E50  operator delete[] wrapper (`if (*p) FUN_0091c6aa(*p)`)
struct offsets touched (top 10):
  +0x4 [dec 4]  x34  via EAX,ECX,EDX
  +0x8 [dec 8]  x34  via EAX,ECX,EDX
  +0x10 [dec 16]  x20  via EAX,ECX,EDX
  +0x38 [dec 56]  x13  via EAX,ECX,EDX
  +0x104 [dec 260]  x10  via EDX
  +0xC [dec 12]  x6  via EDX
  +0x64 [dec 100]  x5  via EAX,ECX,EDX
  +0x14 [dec 20]  x3  via EAX,ECX,EDX
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0x18 [dec 24]  x2  via ECX,EDX
```

### Static facts (script output, report-only)
- size 8676 B, RET 40
- callers 3: 0x008c2450, 0x008c4b50, 0x008c5740
- callees: 0x00517980 (CALL), 0x005142f0 (CALL), 0x00515620 (CALL), 0x005143c0 (CALL), 0x00465560 (CALL), 0x004491c0 (CALL), ...
- indirect: 0x008c39b8 slot 0xc, 0x008c39c9 slot 0xc, 0x008c39e1 slot 0xc, 0x008c3a06 slot 0x8, 0x008c3a58 slot 0xc, ...
- globals: 0x00a1b4a4 READ, 0x00a1b4a4 READ
- twins: none

## Card B17-4-03

GOG function: CSWGuiExamine::HandleInputEvent  gog 0x005b3580  callconv=(blank)  param_size_bytes=8  gog_size=117  gog_retn=8
Candidate Steam address: 0x008bed80 (size 117, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callees: HideExamineBox 0x7cac10); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008bee10:STR0/CALLEE0/SIG0.059/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005b3550 CSWGuiTutorialBox::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005b3610 CSWGuiCreateItemSubMenu::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam 0x0073f750  in candidate callees: yes
  0x004db310 CGuiInGame::HideExamineBox -> Steam 0x007cac10  in candidate callees: yes
  0x0052ced0 CSWGuiMessageBox::HandleInputEvent -> Steam ?  in candidate callees: n/a
  0x00910720 CSWGuiManager::PlayGuiSound -> Steam 0x004122a0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x1c*} | Steam {0x4*,0x1c*}
imm sets (* = in both, 4 common): GOG {0x7*,0x8*,0xc*,0x27*} | Steam {0x7*,0x8*,0xc*,0x27*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x008bed80: 16-byte aligned: YES
  008bed80  55             PUSH EBP
  008bed81  8B EC          MOV EBP,ESP
  008bed83  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008bed80
string literals (0):
direct callees (4):
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x0073F750  CClientExoApp::GetInGameGui
  0x007CAC10  CGuiInGame::HideExamineBox
  0x0075B8E0  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x1  via EDX
  +0x1C [dec 28]  x1  via ECX
  +0x8BEE00 [dec 9170432]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 008bed80 FUN_008bed80 size=117 ret=8 callers=0 callees=4

void FUN_008bed80(int param_1,int param_2)

{
  int iVar1;
  undefined4 extraout_var;
  
  if (param_2 != 0) {
    iVar1 = param_1 + -0x27;
    switch(param_1) {
    case 0x27:
    case 0x28:
    case 0x2d:
    case 0x2e:
      FUN_004122a0(0);
      FUN_0073f750(iVar1,extraout_var,*(undefined4 *)(DAT_00a1b4a4 + 4));
      FUN_007cac10();
      return;
    }
  }
  FUN_0075b8e0(param_1,param_2);
  return;
}
```

### decoy: 0x008bee10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008bee10: 16-byte aligned: YES
  008bee10  55             PUSH EBP
  008bee11  8B EC          MOV EBP,ESP
  008bee13  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x008bee10
string literals (4):
  0x009A8200  at 008beeba  'debug_p'
  0x009A18B8  at 008beed7  'LB_OPTIONS'
  0x009A81F4  at 008bef0b  'LBL_BUILD'
  0x009A81EC  at 008bef4a  'Build: '
direct callees (14):
  0x0040E5A0  CSWGuiPanel::Constructor
  0x0041CE90  (unnamed)
  0x00419740  LabelCtor
  0x0084A640  (unnamed)
  0x0073A9A0  (unnamed)
  0x00710840  (unnamed)
  0x0040EE40  CSWGuiPanel::StartLoadFromLayout
  0x00733570  CExoString::CExoString_char
  0x0040F620  CSWGuiPanel::InitControl
  0x00733780  CExoString::~CExoString
  0x0040F5A0  CSWGuiPanel::StopLoadFromLayout
  0x00733DF0  CExoString::operator+
  0x00416E30  CSWGuiTextParams::SetText
  0x0040EA00  CSWGuiPanel::CenterPanel
struct offsets touched (top 1):
  +0x4A0 [dec 1184]  x1  via EDX
```

### Static facts (script output, report-only)
- size 117 B, RET 8
- callees: 0x004122a0 CSWGuiManager::PlayGuiSound (CALL), 0x0073f750 CClientExoApp::GetInGameGui (CALL), 0x007cac10 (CALL), 0x0075b8e0 (CALL)
- twins: none

## Card B17-4-04

GOG function: CSWGuiBlackenedLabel::Destructor  gog 0x005977a0  callconv=__thiscall  param_size_bytes=0  gog_size=113  gog_retn=0
Candidate Steam address: 0x0074ac30 (size 113, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 4 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 4 (callers: ? 0x74ad90, Destructor_2 0x84df70, Destructor_2 0x8a7060)
- best decoy (neighbour/other candidate) scores: 0x0074ac10:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: autolabel:~CMFCToolBarButtonsListButton
- GOG-order neighbour before: 0x00597770 CSWGuiInGameCharacter::Destructor -> 0x0084def0  trusted
- GOG-order neighbour after:  0x00597820 CSWGuiSliderReadOnly::Destructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x004ce500 CSWGuiControl::Destructor -> Steam 0x00418640  in candidate callees: yes
  0x00913030 CSWGuiBorder::Destructor_2 -> Steam 0x00414c60  in candidate callees: yes
  0x00915660 CSWGuiText::Destructor_2 -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (4 of 901): the card's A-CALLEE credit may come from these, not from callees
  0x0051c510 CSWGuiMainInterface::Destructor_2 -> Steam 0x0074ad90  in candidate callers: yes
  0x00597870 CSWGuiInGameCharacter::Destructor_2 -> Steam 0x0084df70  in candidate callers: yes
  0x0059af60 CSWGuiInGameInventory::Destructor_2 -> Steam 0x008a7060  in candidate callers: yes
  0x005e5240 CSWGuiInGameGameplay::Destructor_2 -> Steam 0x008e07b0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x60*,0xd8*} | Steam {0x60*,0xd8*}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0074ac30: 16-byte aligned: YES
  0074ac30  55             PUSH EBP
  0074ac31  8B EC          MOV EBP,ESP
  0074ac33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0074ac30
string literals (0):
direct callees (3):
  0x00417290  (unnamed)
  0x00414C60  CSWGuiBorder::Destructor_2
  0x00418640  CSWGuiControl::Destructor
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0074ac30 CMFCToolBarButtonsListButton::~CMFCToolBarButtonsListButton size=113 ret=0 callers=901 callees=3

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    public: virtual __thiscall CMFCToolBarButtonsListButton::~CMFCToolBarButtonsListButton(void)
   
   Library: Visual Studio 2010 Debug */

void __thiscall
CMFCToolBarButtonsListButton::~CMFCToolBarButtonsListButton(CMFCToolBarButtonsListButton *this)

{
  uint uVar1;
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  puStack_c = &LAB_00948a53;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *(undefined ***)this = CSWGuiLabel::vftable;
  local_8 = 1;
  FUN_00417290(uVar1);
  local_8 = local_8 & 0xffffff00;
  FUN_00414c60();
  local_8 = 0xffffffff;
  FUN_00418640();
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x0074ac10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0074ac10: 16-byte aligned: YES
  0074ac10  55             PUSH EBP
  0074ac11  8B EC          MOV EBP,ESP
  0074ac13  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0074ac10
string literals (0):
direct callees (1):
  0x00419740  LabelCtor
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 113 B, RET 0
- callers 901: 0x0074ad90, 0x007563b0, 0x007d70d0, 0x0084df70 CSWGuiInGameCharacter::Destructor_2, 0x0088c110, ...
- callees: 0x00417290 (CALL), 0x00414c60 CSWGuiBorder::Destructor_2 (CALL), 0x00418640 CSWGuiControl::Destructor (CALL)
- twins: none

## Card B17-4-05

GOG function: CSWGuiText::GetIdealHeight  gog 0x00915c10  callconv=(blank)  param_size_bytes=0  gog_size=95  gog_retn=0
Candidate Steam address: 0x00417800 (size 61, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=GuiTextObjRefresh | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.08 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00417840:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00915bd0 CSWGuiText::wrapText  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00915c80 CSWGuiText::GetFontHeight -> 0x004178b0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x14*,0x48,0x50} | Steam {0xc,0x10,0x14*,0x4c}
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
prologue for 0x00417800: 16-byte aligned: YES
  00417800  55             PUSH EBP
  00417801  8B EC          MOV EBP,ESP
  00417803  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00417800
string literals (0):
direct callees (0):
struct offsets touched (top 4):
  +0x14 [dec 20]  x3  via EAX,EDX
  +0xC [dec 12]  x1  via EDX
  +0x10 [dec 16]  x1  via ECX
  +0x4C [dec 76]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00417800 FUN_00417800 size=61 ret=0 callers=6 callees=0

void __fastcall FUN_00417800(int param_1)

{
  if (((*(int *)(param_1 + 0x14) != 0) && (*(int *)(param_1 + 0x10) != 0)) &&
     (*(int *)(param_1 + 0xc) != 0)) {
    (**(code **)(**(int **)(param_1 + 0x14) + 0x4c))(0xffffffff);
  }
  return;
}
```

### decoy: 0x00417840 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00417840: 16-byte aligned: YES
  00417840  55             PUSH EBP
  00417841  8B EC          MOV EBP,ESP
  00417843  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00417840
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x14 [dec 20]  x7  via EAX,ECX
  +0x50 [dec 80]  x2  via EDX
  +0x48 [dec 72]  x1  via EDX
```

### Static facts (script output, report-only)
- size 61 B, RET 0
- callers 6: 0x00416cc0 CSWGuiTextParams::SetFont, 0x00416e30 CSWGuiTextParams::SetText, 0x00416ea0 CSWGuiTextParams::SetStrRef, 0x00416fa0 CSWGuiTextParams::SetAlignment, 0x00417340, ...
- indirect: 0x00417837 slot 0x4c
- twins: none

## Card B17-4-06

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

## Card B17-4-07

GOG function: CAurObject::AnimateExternal  gog 0x00844470  callconv=__thiscall  param_size_bytes=4  gog_size=129  gog_retn=4
Candidate Steam address: 0x00451bd0 (size 271, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.12 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00451a80:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00844430 MaxTree::GetType  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00844500 Global::MaxTreeDestructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 5): the card's A-CALLEE credit may come from these, not from callees
  0x00454660 CSWCArea::SetWeather -> Steam 0x007a7c60  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x14,0x50,0x94} | Steam {0x4,0x8,0xc}
imm sets (* = in both, 1 common): GOG {0x4*,0x8} | Steam {0x4*,0x10,0x28}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00451bd0: 16-byte aligned: YES
  00451bd0  55             PUSH EBP
  00451bd1  8B EC          MOV EBP,ESP
  00451bd3  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00451bd0
string literals (0):
direct callees (2):
  0x0091D288  operator new[]
  0x0091C6AA  operator delete[]
struct offsets touched (top 3):
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0xC [dec 12]  x3  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00451bd0 FUN_00451bd0 size=271 ret=4 callers=3 callees=2

void __thiscall FUN_00451bd0(int *param_1,int param_2)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  undefined4 *local_2c;
  uint local_24;
  undefined4 *local_20;
  int local_c;
  
  iVar1 = *param_1;
  param_1[2 = byte +0x8] = param_2;
  local_24 = param_1[2 = byte +0x8];
  local_2c = (undefined4 *)
             FUN_0091d288(-(uint)((int)((ulonglong)local_24 * 0x10 >> 0x20) != 0) |
                          (uint)((ulonglong)local_24 * 0x10));
  local_20 = local_2c;
  if (local_2c == (undefined4 *)0x0) {
    local_2c = (undefined4 *)0x0;
  }
  else {
    while (local_24 = local_24 - 1, -1 < (int)local_24) {
      local_20[3 = byte +0xc] = 0;
      local_20[2 = byte +0x8] = 0;
      local_20[1 = byte +0x4] = 0;
      *local_20 = 0x3f800000;
      local_20 = local_20 + 0x4 [dec 4] {undefined4*: byte +0x10};
    }
  }
  *param_1 = (int)local_2c;
  for (local_c = 0; local_c < param_1[1 = byte +0x4]; local_c = local_c + 1) {
    puVar2 = (undefined4 *)(local_c * 0x10 + iVar1);
    puVar3 = (undefined4 *)(*param_1 + local_c * 0x10);
    *puVar3 = *puVar2;
    puVar3[1 = byte +0x4] = puVar2[1 = byte +0x4];
    puVar3[2 = byte +0x8] = puVar2[2 = byte +0x8];
    puVar3[3 = byte +0xc] = puVar2[3 = byte +0xc];
  }
  if (iVar1 != 0) {
    FUN_0091c6aa(iVar1);
  }
  return;
}
```

### decoy: 0x00451a80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00451a80: 16-byte aligned: YES
  00451a80  55             PUSH EBP
  00451a81  8B EC          MOV EBP,ESP
  00451a83  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00451a80
string literals (0):
direct callees (4):
  0x0091D288  operator new[]
  0x0091D67E  (unnamed)
  0x004741E0  SafePointer_T_::operator=
  0x004518C0  (unnamed)
struct offsets touched (top 2):
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via ECX
```

### Static facts (script output, report-only)
- size 271 B, RET 4
- callers 3: 0x00451800, 0x00451840, 0x00463e70
- callees: 0x0091d288 operator new[] (CALL), 0x0091c6aa operator delete[] (CALL)
- twins: none

## Card B17-4-08

GOG function: Global::ImageGetSubImage  gog 0x008d8c90  callconv=(blank)  param_size_bytes=8  gog_size=176  gog_retn=0
Candidate Steam address: 0x004841b0 (size 516, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00484100:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x008d8a80 Global::ImageLowpassBlur  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008d8d40 Global::AurImageApplyLowEndEnvMap  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
  0x00925530 Global::memcpy -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x1,0x2,0x3,0x4,0x8}
imm sets (* = in both, 2 common): GOG {0x4*,0xc*,0x20} | Steam {0x4*,0xc*,0xa8}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 0)
```

### prologue
```
prologue for 0x004841b0: 16-byte aligned: YES
  004841b0  55             PUSH EBP
  004841b1  8B EC          MOV EBP,ESP
  004841b3  81 EC A8 00 00 00 SUB ESP,0xa8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004841b0
string literals (0):
direct callees (2):
  0x0045CFE0  (unnamed)
  0x0044C8A0  Vector::operator+ (Vector3 add)
struct offsets touched (top 5):
  +0x1 [dec 1]  x2  via EAX,ECX
  +0x2 [dec 2]  x2  via EAX,EDX
  +0x3 [dec 3]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 004841b0 FUN_004841b0 size=516 ret=0 callers=1 callees=2

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_004841b0(int param_1,int param_2,int param_3,int param_4,undefined4 param_5)

{
  undefined4 uVar1;
  float *pfVar2;
  undefined1 local_5c [12];
  undefined1 local_50 [12];
  undefined1 local_44 [12];
  int local_38;
  float local_34;
  float local_30;
  float local_2c;
  int local_28;
  undefined1 local_24;
  undefined1 local_20;
  float local_1c;
  float local_18;
  float local_14;
  float local_10;
  int local_c;
  undefined1 local_8;
  
  if (param_4 == 4) {
    local_34 = 0.0;
    local_30 = 0.0;
    local_2c = 0.0;
    for (local_c = 0; local_c < param_3; local_c = local_c + 1) {
      for (local_38 = 0; local_38 < param_2; local_38 = local_38 + 1) {
        local_28 = (local_c * param_2 + local_38) * 4;
        local_1c = (float)*(byte *)(param_1 + local_28) / (float)_DAT_009884a0;
        local_18 = (float)*(byte *)(param_1 + local_28 + 1) / (float)_DAT_009884a0;
        local_14 = (float)*(byte *)(param_1 + local_28 + 2) / (float)_DAT_009884a0;
        local_10 = (float)*(byte *)(param_1 + local_28 + 3) / (float)_DAT_009884a0;
        uVar1 = FUN_0045cfe0(local_44,1.0 - local_10,param_5);
        uVar1 = FUN_0045cfe0(local_50,local_10,&local_1c,uVar1);
        pfVar2 = (float *)FUN_0044c8a0(local_5c,uVar1);
        local_34 = *pfVar2;
        local_30 = pfVar2[1 = byte +0x4];
        local_2c = pfVar2[2 = byte +0x8];
        local_8 = (undefined1)(int)ROUND(ROUND(local_34 * (float)_DAT_009884a0));
        local_24 = (undefined1)(int)ROUND(ROUND(local_30 * (float)_DAT_009884a0));
        local_20 = (undefined1)(int)ROUND(ROUND(local_2c * (float)_DAT_009884a0));
        *(undefined1 *)(param_1 + local_28) = local_8;
        *(undefined1 *)(param_1 + local_28 + 1) = local_24;
        *(undefined1 *)(param_1 + local_28 + 2) = local_20;
        *(undefined1 *)(param_1 + local_28 + 3) = 0xff;
      }
    }
  }
  return;
}
```

### decoy: 0x00484100 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00484100: 16-byte aligned: YES
  00484100  55             PUSH EBP
  00484101  8B EC          MOV EBP,ESP
  00484103  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00484100
string literals (0):
direct callees (2):
  0x00919723  operator new
  0x0091DBA0  _memcpy
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 516 B, RET 0
- callers 1: 0x00426270 CAurTextureBasic::ConstructImage
- callees: 0x0045cfe0 (CALL), 0x0044c8a0 (CALL)
- twins: none

## Card B17-4-09

GOG function: CSWGuiPartySelection::OnToggled  gog 0x0058c660  callconv=(blank)  param_size_bytes=4  gog_size=567  gog_retn=4
Candidate Steam address: 0x0089eb50 (size 567, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.44) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0089ed90:STR0/CALLEE0/SIG0.302/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0058c530 CSWGuiPartySelection::OnPanelRemoved  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0058c8a0 CSWGuiPartySelection::OnDone  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam 0x0073f750  in candidate callees: yes
  0x0052d090 CSWGuiMessageBox::SetAllowCancel -> Steam ?  in candidate callees: n/a
  0x0052e030 CSWGuiMessageBox::SetCallback -> Steam ?  in candidate callees: n/a
  0x0058a930 CSWGuiPartySelectionButton::SetSelected -> Steam ?  in candidate callees: n/a
  0x0058aa20 CSWGuiPartySelectionButton::GetSelected -> Steam ?  in candidate callees: n/a
  0x0058d520 CSWGuiPartySelection::UpdateCount -> Steam ?  in candidate callees: n/a
  0x0058e370 CSWGuiPartySelection::SetAddRemoveLabel -> Steam ?  in candidate callees: n/a
  0x0090e9b0 CSWGuiManager::AddPanel -> Steam 0x00410530  in candidate callees: yes
  ... 1 more GOG callees
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 13 common): GOG {0x4*,0x1c*,0x4c*,0x50*,0x58*,0x5c*,0x6c*,0x70*,0xa0*,0x468*,0x48f8*,0x50ac*,0x50b0*} | Steam {0x4*,0x1c*,0x4c*,0x50*,0x58*,0x5c*,0x6c*,0x70*,0xa0*,0x468*,0x48f8*,0x50ac*,0x50b0*}
imm sets (* = in both, 8 common): GOG {0x2*,0x4*,0x2d*,0x3c*,0x4728*,0xa588*,0xa5a6*,0x1a789*} | Steam {0x2*,0x4*,0x2d*,0x3c*,0x4728*,0xa588*,0xa5a6*,0x1a789*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0089eb50: 16-byte aligned: YES
  0089eb50  55             PUSH EBP
  0089eb51  8B EC          MOV EBP,ESP
  0089eb53  83 EC 3C       SUB ESP,0x3c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0089eb50
string literals (0):
direct callees (9):
  0x0073F750  CClientExoApp::GetInGameGui
  0x0075BB00  (unnamed)
  0x0075CA30  (unnamed)
  0x00410530  CSWGuiManager::AddPanel
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x0089CF10  (unnamed)
  0x0089CE20  (unnamed)
  0x008A0860  (unnamed)
  0x0089FA10  (unnamed)
struct offsets touched (top 10):
  +0x6C [dec 108]  x5  via EAX,ECX,EDX
  +0x70 [dec 112]  x4  via EDX
  +0x1C [dec 28]  x3  via ECX,EDX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0xA0 [dec 160]  x2  via EAX,EDX
  +0x468 [dec 1128]  x2  via ECX,EDX
  +0x48F8 [dec 18680]  x2  via EAX,EDX
  +0x4C [dec 76]  x1  via ECX
  +0x50 [dec 80]  x1  via EAX
  +0x58 [dec 88]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0089eb50 FUN_0089eb50 size=567 ret=4 callers=0 callees=9

void __thiscall FUN_0089eb50(int param_1,int param_2)

{
  int *piVar1;
  int iVar2;
  int local_8;
  
  if (*(int *)(param_2 + 0x50) != 0) {
    if (*(int *)(param_1 + 0x50ac) == 0) {
      iVar2 = FUN_0073f750();
      piVar1 = *(int **)(iVar2 + 0xa0);
      FUN_0075bb00(0);
      FUN_0075ca30(0,0,0);
      (**(code **)(*piVar1 + 0x70))(*(undefined4 *)(param_1 + 0x50b0));
      FUN_00410530(piVar1,1,1);
    }
    else {
      if (*(int *)(param_2 + 0x4c) == 0x2d) {
        FUN_004122a0(*(undefined1 *)(param_2 + 0x58));
      }
      if (param_2 == param_1 + 0x4728) {
        local_8 = *(int *)(param_1 + 0x48f8);
        if (local_8 == 0) {
          return;
        }
      }
      else {
        local_8 = *(int *)(param_2 + 0x5c);
      }
      if ((*(uint *)(local_8 + 0x468) & 1) == 0) {
        iVar2 = FUN_0073f750();
        piVar1 = *(int **)(iVar2 + 0xa0);
        FUN_0075bb00(0);
        FUN_0075ca30(0,0,0);
        if ((*(uint *)(local_8 + 0x468) >> 2 & 1) == 0) {
          iVar2 = FUN_0089cf10();
          if (iVar2 == 0) {
            (**(code **)(*piVar1 + 0x70))(0xa588);
          }
          else {
            (**(code **)(*piVar1 + 0x70))(0x1a789);
          }
        }
        else {
          (**(code **)(*piVar1 + 0x70))(0xa5a6);
        }
        FUN_00410530(piVar1,1,1);
      }
      else {
        iVar2 = FUN_0089cf10();
        if (iVar2 == 0) {
          if (*(int *)(param_1 + 0x6c) < 2) {
            *(int *)(param_1 + 0x6c) = *(int *)(param_1 + 0x6c) + 1;
            FUN_0089ce20(1);
          }
        }
        else {
          *(int *)(param_1 + 0x6c) = *(int *)(param_1 + 0x6c) + -1;
          FUN_0089ce20(0);
        }
        iVar2 = FUN_0089cf10();
        FUN_008a0860(iVar2 == 0);
        FUN_0089fa10();
      }
    }
  }
  return;
}
```

### decoy: 0x0089ed90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0089ed90: 16-byte aligned: YES
  0089ed90  55             PUSH EBP
  0089ed91  8B EC          MOV EBP,ESP
  0089ed93  83 EC 38       SUB ESP,0x38
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0089ed90
string literals (0):
direct callees (10):
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x0089CF10  (unnamed)
  0x0089FBD0  (unnamed)
  0x0073FEE0  CClientExoApp::SetInputClass
  0x0073F750  CClientExoApp::GetInGameGui
  0x0075BAA0  (unnamed)
  0x0075BB00  (unnamed)
  0x0075CA30  (unnamed)
  0x00410530  CSWGuiManager::AddPanel
  0x0089CE20  (unnamed)
struct offsets touched (top 9):
  +0x4 [dec 4]  x3  via ECX,EDX
  +0x1C [dec 28]  x2  via EAX
  +0x4C [dec 76]  x2  via ECX,EDX
  +0x70 [dec 112]  x2  via EAX,EDX
  +0x30 [dec 48]  x1  via EAX
  +0x50 [dec 80]  x1  via EAX
  +0x74 [dec 116]  x1  via EDX
  +0xA0 [dec 160]  x1  via ECX
  +0x50AC [dec 20652]  x1  via ECX
```

### Static facts (script output, report-only)
- size 567 B, RET 4
- callees: 0x0073f750 CClientExoApp::GetInGameGui (CALL), 0x0075bb00 (CALL), 0x0075ca30 (CALL), 0x00410530 CSWGuiManager::AddPanel (CALL), 0x004122a0 CSWGuiManager::PlayGuiSound (CALL), 0x0089cf10 (CALL), ...
- indirect: 0x0089ebc7 slot 0x70, 0x0089ed32 slot 0x70, 0x0089ed52 slot 0x70, 0x0089ed66 slot 0x70
- twins: none

## Card B17-4-10

GOG function: CSWGuiContainer::Destructor_2  gog 0x005a61c0  callconv=(blank)  param_size_bytes=0  gog_size=560  gog_retn=0
Candidate Steam address: 0x008b2620 (size 560, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x008b2850:STR0/CALLEE0/SIG0.044/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005a6190 CSWGuiContainer::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005a63f0 CSWGuiContainer::HandleInputEvent  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x004ce500 CSWGuiControl::Destructor -> Steam 0x00418640  in candidate callees: yes
  0x004ce630 CSWGuiButton::Destructor -> Steam 0x0041a3d0  in candidate callees: yes
  0x0090cce0 CSWGuiPanel::Destructor -> Steam 0x0040e860  in candidate callees: yes
  0x00913030 CSWGuiBorder::Destructor_2 -> Steam 0x00414c60  in candidate callees: yes
  0x00915660 CSWGuiText::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x0091b1a0 CSWGuiListBox::Destructor_2 -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 33 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+19)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+19)}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x008b2620: 16-byte aligned: YES
  008b2620  55             PUSH EBP
  008b2621  8B EC          MOV EBP,ESP
  008b2623  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x008b2620
string literals (0):
direct callees (6):
  0x0041A3D0  CSWGuiButton::Destructor
  0x0041D190  (unnamed)
  0x00417290  (unnamed)
  0x00414C60  CSWGuiBorder::Destructor_2
  0x00418640  CSWGuiControl::Destructor
  0x0040E860  CSWGuiPanel::Destructor
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 008b2620 FUN_008b2620 size=560 ret=0 callers=1 callees=6

void __fastcall FUN_008b2620(int param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_00978ea5;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_8 = 9;
  FUN_0041a3d0(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8._0_1_ = 8;
  FUN_0041a3d0();
  local_8._0_1_ = 7;
  FUN_0041a3d0();
  local_8._0_1_ = 6;
  FUN_0041d190();
  *(undefined ***)(param_1 + 0x6dc) = CSWGuiLabel::vftable;
  local_8._0_1_ = 0xb;
  FUN_00417290();
  local_8._0_1_ = 10;
  FUN_00414c60();
  local_8._0_1_ = 5;
  FUN_00418640();
  *(undefined ***)(param_1 + 0x594) = CSWGuiLabel::vftable;
  local_8._0_1_ = 0xd;
  FUN_00417290();
  local_8._0_1_ = 0xc;
  FUN_00414c60();
  local_8._0_1_ = 4;
  FUN_00418640();
  *(undefined ***)(param_1 + 0x44c) = CSWGuiLabel::vftable;
  local_8._0_1_ = 0xf;
  FUN_00417290();
  local_8._0_1_ = 0xe;
  FUN_00414c60();
  local_8._0_1_ = 3;
  FUN_00418640();
  *(undefined ***)(param_1 + 0x304) = CSWGuiLabel::vftable;
  local_8._0_1_ = 0x11;
  FUN_00417290();
  local_8._0_1_ = 0x10;
  FUN_00414c60();
  local_8._0_1_ = 2;
  FUN_00418640();
  *(undefined ***)(param_1 + 0x1bc) = CSWGuiLabel::vftable;
  local_8._0_1_ = 0x13;
  FUN_00417290();
  local_8._0_1_ = 0x12;
  FUN_00414c60();
  local_8._0_1_ = 1;
  FUN_00418640();
  *(undefined ***)(param_1 + 0x74) = CSWGuiLabel::vftable;
  local_8._0_1_ = 0x15;
  FUN_00417290();
  local_8._0_1_ = 0x14;
  FUN_00414c60();
  local_8 = (uint)local_8._1_3_ << 8;
  FUN_00418640();
  local_8 = 0xffffffff;
  FUN_0040e860();
  ExceptionList = local_10;
  return;
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
- size 560 B, RET 0
- callers 1: 0x008b25f0
- callees: 0x0041a3d0 CSWGuiButton::Destructor (CALL), 0x0041d190 (CALL), 0x00417290 (CALL), 0x00414c60 CSWGuiBorder::Destructor_2 (CALL), 0x00418640 CSWGuiControl::Destructor (CALL), 0x0040e860 CSWGuiPanel::Destructor (CALL)
- twins: none

## Card B17-4-11

GOG function: Global::CullUnnecessaryKeys  gog 0x008c68e0  callconv=(blank)  param_size_bytes=4  gog_size=874  gog_retn=0
Candidate Steam address: 0x004861d0 (size 445, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.42 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00486390:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x008c6720 Global::AddGenericControllerSingle  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008c6c50 Global::AddGenericController  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x008cebd0 unnamed -> Steam ?  in candidate callees: n/a
  0x00925c49 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0xc*,0x10} | Steam {0x4*,0x8*,0xc*}
imm sets (* = in both, 1 common): GOG {0x2,0x8*,0x2c,0x41,0xa4} | Steam {0x3,0x4,0x8*,0xc,0x10,0x58}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x004861d0: 16-byte aligned: YES
  004861d0  55             PUSH EBP
  004861d1  8B EC          MOV EBP,ESP
  004861d3  83 EC 58       SUB ESP,0x58
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004861d0
string literals (0):
direct callees (4):
  0x004850B0  (unnamed)
  0x005195A0  (unnamed)
  0x00451660  (unnamed)
  0x0048E460  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0xC [dec 12]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 004861d0 FUN_004861d0 size=445 ret=0 callers=4 callees=4

void FUN_004861d0(undefined4 param_1,undefined4 param_2,undefined4 param_3,int param_4,
                 undefined4 param_5,undefined2 param_6)

{
  undefined2 uVar1;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38[10 = byte +0x28];
  int local_10;
  int local_c;
  
  local_44 = CONCAT22(1,param_6);
  FUN_004850b0(param_2,local_38);
  if (local_10 == 4) {
    if ((DAT_00a33234 & 1) == 0) {
      DAT_00a33234 = DAT_00a33234 | 1;
    }
    DAT_00a33228 = local_38[0];
    DAT_00a3322c = local_38[1 = byte +0x4];
    DAT_00a33230 = local_38[2 = byte +0x8];
    FUN_005195a0(local_38[0],local_38[1 = byte +0x4],local_38[2 = byte +0x8],local_38[3 = byte +0xc]);
  }
  for (local_c = local_10 + -1; -1 < local_c; local_c = local_c + -1) {
    local_38[local_c + 1] = local_38[local_c];
  }
  local_38[0] = 0;
  if (local_10 == 1) {
    local_3c = 1;
  }
  else if (local_10 == 3) {
    local_3c = 3;
  }
  else if (local_10 == 4) {
    local_3c = 4;
  }
  uVar1 = *(undefined2 *)(param_4 + 4);
  local_10 = local_10 + 1;
  FUN_00451660(0);
  local_40 = CONCAT22(*(undefined2 *)(param_4 + 4),uVar1);
  for (local_c = 1; local_c < local_10; local_c = local_c + 1) {
    FUN_00451660(local_38[local_c]);
  }
  FUN_0048e460(param_5,local_44,local_40,local_3c);
  DAT_00a32e08 = DAT_00a32e08 + 1;
  return;
}
```

### decoy: 0x00486390 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00486390: 16-byte aligned: YES
  00486390  55             PUSH EBP
  00486391  8B EC          MOV EBP,ESP
  00486393  81 EC A4 00 00 00 SUB ESP,0xa4
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00486390
string literals (0):
direct callees (2):
  0x0091EBAD  (unnamed)
  0x0048E680  (unnamed)
struct offsets touched (top 4):
  +0x4 [dec 4]  x5  via EAX,ECX,EDX
  +0x8 [dec 8]  x4  via ECX,EDX
  +0xC [dec 12]  x4  via ECX,EDX
  +0x10 [dec 16]  x4  via ECX,EDX
```

### Static facts (script output, report-only)
- size 445 B, RET 0
- callers 4: 0x00486aa0, 0x00486fb0, 0x004878b0, 0x0048b6f0
- callees: 0x004850b0 (CALL), 0x005195a0 (CALL), 0x00451660 (CALL), 0x0048e460 (CALL)
- twins: none

## Card B17-4-12

GOG function: Global::ProceduralAnimateHierarchy  gog 0x008f9cd0  callconv=(blank)  param_size_bytes=4  gog_size=815  gog_retn=0
Candidate Steam address: 0x004dfed0 (size 614, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.16 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004dfba0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x008f9290 MdlNodeAnimMesh::InternalControl  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008fa000 Global::AnimateHierarchy -> 0x004dfed0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x008255f0 GLRender::LockPool -> Steam ?  in candidate callees: n/a
  0x00825620 GLRender::UnlockPool -> Steam ?  in candidate callees: n/a
  0x00844e40 MaxTree::AsModel -> Steam ?  in candidate callees: n/a
  0x00846bb0 MdlNode::AsMdlNodeTriMesh -> Steam ?  in candidate callees: n/a
  0x008f9cd0 Global::ProceduralAnimateHierarchy -> Steam ?  in candidate callees: n/a
  0x00924490 unnamed -> Steam ?  in candidate callees: n/a
  0x00925936 Global::rand -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x4*,0x8*,0x2c*,0x30*,0x34*,0x78,0x90,0xac,0x138,0x13c,0x140,0x144,0x148,0x14c,...(+3)} | Steam {0x2,0x4*,0x8*,0x16,0x28,0x2c*,0x30*,0x34*,0x40,0x48,0x58,0xa8,0x164}
imm sets (* = in both, 1 common): GOG {0x4,0x5*,0x10,0x44} | Steam {0x5*,0x18,0x28,0x41,0x64}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x004dfed0: 16-byte aligned: YES
  004dfed0  55             PUSH EBP
  004dfed1  8B EC          MOV EBP,ESP
  004dfed3  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004dfed0
string literals (0):
direct callees (4):
  0x00452830  (unnamed)
  0x00453860  MdlNode::ControlDispatch
  0x00459E20  Gob::IsInCutscene
  0x004DFED0  (unnamed)
struct offsets touched (top 10):
  +0x30 [dec 48]  x5  via ECX,EDX
  +0x4 [dec 4]  x2  via ECX
  +0x28 [dec 40]  x2  via ECX,EDX
  +0x2C [dec 44]  x2  via EAX,EDX
  +0x58 [dec 88]  x2  via ECX,EDX
  +0x164 [dec 356]  x2  via EAX,ECX
  +0x2 [dec 2]  x1  via EAX
  +0x8 [dec 8]  x1  via EDX
  +0x16 [dec 22]  x1  via EAX
  +0x34 [dec 52]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 004dfed0 FUN_004dfed0 size=614 ret=0 callers=2 callees=4

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_004dfed0(int param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,
                 char param_6)

{
  char cVar1;
  int iVar2;
  float local_2c;
  int local_14;
  int local_10;
  int local_8;
  
  DAT_00a7bc20 = DAT_00a7bc20 + 1;
  if (DAT_00a7bc24 < DAT_00a7bc20) {
    DAT_00a7bc24 = DAT_00a7bc20;
  }
  if (((param_6 == '\0') && (param_1 != 0)) &&
     (iVar2 = FUN_00452830(), *(int *)(param_1 + 4) == *(int *)(iVar2 + 0xa8))) {
    param_6 = '\x01';
  }
  if ((*(int *)(param_1 + 0x48) == 0) && (param_6 != '\0')) {
    FUN_00453860(param_1,param_3,param_4,param_5);
  }
  local_10 = 100;
  iVar2 = *(int *)(param_1 + 0x40);
  if (((iVar2 != 0) && (DAT_00a04a0c != 0)) &&
     ((cVar1 = FUN_00459e20(), cVar1 == '\0' && (*(char *)(iVar2 + 0x16) != '\0')))) {
    if ((float)_DAT_00987f08 <= *(float *)(iVar2 + 0x164)) {
      local_2c = *(float *)(iVar2 + 0x164);
    }
    else {
      local_2c = _DAT_0098c014;
    }
    if (*(float *)(iVar2 + 0x58) <= _DAT_00a04a00 * (local_2c / (float)_DAT_00991350)) {
      if (_DAT_00a049fc * (local_2c / (float)_DAT_00991350) < *(float *)(iVar2 + 0x58)) {
        local_10 = DAT_00a04a08;
      }
    }
    else {
      local_10 = DAT_00a04a04;
    }
  }
  if ((DAT_00a7bc20 < DAT_00a049f8) && (DAT_00a7bc20 < local_10)) {
    for (local_8 = 0; local_8 < *(int *)(param_1 + 0x34); local_8 = local_8 + 1) {
      if (*(int *)(*(int *)(*(int *)(param_1 + 0x30) + local_8 * 4) + 0x28) != -1) {
        for (local_14 = 0; local_14 < *(int *)(param_2 + 0x30); local_14 = local_14 + 1) {
          iVar2 = (local_14 + local_8) % *(int *)(param_2 + 0x30);
          if (*(int *)(*(int *)(*(int *)(param_1 + 0x30) + local_8 * 4) + 0x28) ==
              (int)*(short *)(*(int *)(*(int *)(param_2 + 0x2c) + iVar2 * 4) + 2)) {
            FUN_004dfed0(*(undefined4 *)(*(int *)(param_1 + 0x30) + local_8 * 4),
                         *(undefined4 *)(*(int *)(param_2 + 0x2c) + iVar2 * 4),param_3,param_4,
                         param_5,param_6);
            break;
          }
        }
      }
    }
  }
  DAT_00a7bc20 = DAT_00a7bc20 + -1;
  return;
}
```

### decoy: 0x004dfba0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004dfba0: 16-byte aligned: YES
  004dfba0  55             PUSH EBP
  004dfba1  8B EC          MOV EBP,ESP
  004dfba3  83 EC 44       SUB ESP,0x44
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004dfba0
string literals (0):
direct callees (7):
  0x004545A0  (unnamed)
  0x0091C860  _ftol2_sse
  0x00432EE0  (unnamed)
  0x0091EB8B  (unnamed)
  0x00452830  (unnamed)
  0x00432F10  (unnamed)
  0x004DFBA0  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x12  via EAX,ECX,EDX
  +0x78 [dec 120]  x4  via ECX,EDX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0xAC [dec 172]  x2  via EAX
  +0x144 [dec 324]  x2  via EAX,ECX
  +0x148 [dec 328]  x2  via EAX
  +0x160 [dec 352]  x2  via EAX,ECX
  +0x180 [dec 384]  x2  via EAX,ECX
  +0x2C [dec 44]  x1  via EDX
  +0x30 [dec 48]  x1  via ECX
```

### Static facts (script output, report-only)
- size 614 B, RET 0
- callers 2: 0x004dfed0, 0x004e1d40
- callees: 0x00452830 (CALL), 0x00453860 (CALL), 0x00459e20 Gob::IsInCutscene (CALL), 0x004dfed0 (CALL)
- twins: none

## Card B17-4-13

GOG function: CSWGuiInGameEquip::Draw  gog 0x0059f8a0  callconv=(blank)  param_size_bytes=4  gog_size=209  gog_retn=4
Candidate Steam address: 0x008aba50 (size 209, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callees: Draw 0x40ec90); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008abb30:STR0/CALLEE0/SIG0.174/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0059f6b0 CSWGuiInGameEquip::SetDescription  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0059f980 CSWGuiInGameEquip::OnPanelAdded  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0040d070 CClientExoApp::GetSWParty -> Steam 0x0073fb90  in candidate callees: yes
  0x004789b0 CSWParty::GetPlayerCharacter -> Steam ?  in candidate callees: n/a
  0x0059ec10 CSWGuiInGameEquip::SetCharacter -> Steam ?  in candidate callees: n/a
  0x005a1780 CSWGuiInGameEquip::UpdateInventory -> Steam ?  in candidate callees: n/a
  0x0090d110 CSWGuiPanel::Draw -> Steam 0x0040ec90  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x4*,0x88*,0x4ec4*,0x50e4*,0x50e8*} | Steam {0x4*,0x88*,0x4ec4*,0x50e4*,0x50e8*}
imm sets (* = in both, 4 common): GOG {0x4*,0x8*,0x41*,0x4ec4*} | Steam {0x4*,0x8*,0x41*,0x4ec4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x008aba50: 16-byte aligned: YES
  008aba50  55             PUSH EBP
  008aba51  8B EC          MOV EBP,ESP
  008aba53  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008aba50
string literals (0):
direct callees (5):
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  GetControlledCreature
  0x008AADC0  (unnamed)
  0x008AD930  Equipment panel slot refresh (per frame from 0x8ABA50 via 0x8ABA8C): top placeholder resets ctl0/ctl9, slot loop k=[EBP-0x28] 0..10, slot image ctl = this+0x14D0+k*0x148
  0x0040EC90  CSWGuiPanel::Draw
struct offsets touched (top 5):
  +0x50E4 [dec 20708]  x4  via EAX,ECX,EDX
  +0x88 [dec 136]  x2  via EAX,EDX
  +0x4EC4 [dec 20164]  x2  via EAX,EDX
  +0x4 [dec 4]  x1  via ECX
  +0x50E8 [dec 20712]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 008aba50 FUN_008aba50 size=209 ret=4 callers=0 callees=5

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_008aba50(int param_1,float param_2)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 0x50e8) == 0) {
    FUN_0073fb90(param_1,*(undefined4 *)(DAT_00a1b4a4 + 4));
    uVar1 = FUN_007e5d80();
    FUN_008aadc0(uVar1);
  }
  FUN_008ad930();
  if ((float)_DAT_00986b38 < *(float *)(param_1 + 0x50e4)) {
    *(float *)(param_1 + 0x50e4) = *(float *)(param_1 + 0x50e4) - param_2;
  }
  if (*(float *)(param_1 + 0x50e4) <= (float)_DAT_00986b38) {
    (**(code **)(*(int *)(param_1 + 0x4ec4) + 0x88))(1);
  }
  else {
    (**(code **)(*(int *)(param_1 + 0x4ec4) + 0x88))(0);
  }
  FUN_0040ec90(param_2);
  return;
}
```

### decoy: 0x008abb30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008abb30: 16-byte aligned: YES
  008abb30  55             PUSH EBP
  008abb31  8B EC          MOV EBP,ESP
  008abb33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x008abb30
string literals (1):
  0x0099E4C4  at 008abb76  '003EBO'
direct callees (11):
  0x0051BDB0  CServerExoApp::GetModuleName
  0x00734F60  CExoString::CompareNoCase(char*)
  0x0073F750  CClientExoApp::GetInGameGui
  0x007D2220  CGuiInGame::SetTopMenuOverrideCharacter
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  GetControlledCreature
  0x008AADC0  (unnamed)
  0x008B0890  (unnamed)
  0x008AC330  (unnamed)
  0x0040F3C0  CSWGuiPanel::OnPanelAdded
  0x00733780  CExoString::~CExoString
struct offsets touched (top 6):
  +0x48 [dec 72]  x12  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x50E8 [dec 20712]  x2  via EAX,ECX
  +0x50EC [dec 20716]  x2  via ECX,EDX
  +0x8 [dec 8]  x1  via EAX
  +0x20 [dec 32]  x1  via EDX
```

### Static facts (script output, report-only)
- size 209 B, RET 4
- callees: 0x0073fb90 CClientExoApp::GetSWParty (CALL), 0x007e5d80 (CALL), 0x008aadc0 (CALL), 0x008ad930 Equipment panel slot refresh (per frame from 0x8ABA50 via 0x8ABA8C): top placeholder resets ctl0/ctl9, slot loop k=[EBP-0x28] 0..10, slot image ctl = this+0x14D0+k*0x148 (CALL), 0x0040ec90 CSWGuiPanel::Draw (CALL)
- indirect: 0x008abaec slot 0x88, 0x008abb0a slot 0x88
- twins: none

## Card B17-4-14

GOG function: CSWGuiCreateMedicalItem::DeleteItemEntries  gog 0x005ca130  callconv=(blank)  param_size_bytes=0  gog_size=108  gog_retn=4
Candidate Steam address: 0x008d9d60 (size 108, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: rotation:0x005ca1a0:0.56>0.19
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008d98f0:STR0/CALLEE0/SIG0.147/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x005c9cc0 CSWGuiCreateMedicalItem::OnControlStoreAButton  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005ca1a0 CSWGuiCreateMedicalItem::OnPanelRemoved_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x005cb540 CSWGuiCreateMedicalItem::ShowSellGUI -> Steam ?  in candidate callees: n/a
  0x005cb770 CSWGuiCreateMedicalItem::ShowBuyGUI -> Steam ?  in candidate callees: n/a
  0x00910720 CSWGuiManager::PlayGuiSound -> Steam 0x004122a0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x1c*,0x2b54*,0x33f8*} | Steam {0x1c*,0x2b54*,0x33f8*}
imm sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 4)
```

### prologue
```
prologue for 0x008d9d60: 16-byte aligned: YES
  008d9d60  55             PUSH EBP
  008d9d61  8B EC          MOV EBP,ESP
  008d9d63  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008d9d60
string literals (0):
direct callees (3):
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x008DB170  (unnamed)
  0x008DB3A0  (unnamed)
struct offsets touched (top 3):
  +0x33F8 [dec 13304]  x3  via EAX,ECX,EDX
  +0x1C [dec 28]  x1  via EAX
  +0x2B54 [dec 11092]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 008d9d60 FUN_008d9d60 size=108 ret=4 callers=1 callees=3

void __fastcall FUN_008d9d60(int param_1)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 0x33f8) == 0) {
    *(undefined4 *)(param_1 + 0x33f8) = 1;
  }
  else {
    *(undefined4 *)(param_1 + 0x33f8) = 0;
  }
  uVar1 = *(undefined4 *)(param_1 + 0x1c);
  FUN_004122a0(0);
  if ((*(uint *)(param_1 + 0x2b54) >> 1 & 1) == 0) {
    FUN_008db3a0(param_1,uVar1);
  }
  else {
    FUN_008db170();
  }
  return;
}
```

### decoy: 0x008d98f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008d98f0: 16-byte aligned: YES
  008d98f0  55             PUSH EBP
  008d98f1  8B EC          MOV EBP,ESP
  008d98f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x008d98f0
string literals (0):
direct callees (21):
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x0073F750  CClientExoApp::GetInGameGui
  0x0075BB00  (unnamed)
  0x0075CA30  (unnamed)
  0x0075BBE0  (unnamed)
  0x0073FEA0  CClientExoApp::GetSWGuiManager
  0x00410530  CSWGuiManager::AddPanel
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x0051C0E0  CServerExoApp::GetItemByGameObjectID
  0x008DB5E0  (unnamed)
  0x0051C8F0  CServerExoApp::GetPlayerCreatureId
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006A4E00  CSWSCreatureStats::GetLevel
  0x00733540  CExoString::Constructor
  0x00733700  CExoString::Constructor_5
  0x0071E910  (unnamed)
  0x00733780  CExoString::~CExoString
  0x00619C90  (unnamed)
  0x007337C0  CExoString::operator=
  0x007335F0  CExoString::CExoString_copy
struct offsets touched (top 10):
  +0x4 [dec 4]  x12  via EAX,ECX,EDX
  +0x8 [dec 8]  x9  via EAX,ECX,EDX
  +0xA0 [dec 160]  x3  via ECX,EDX
  +0x70 [dec 112]  x2  via EAX,EDX
  +0x1C [dec 28]  x1  via ECX
  +0x4C [dec 76]  x1  via EDX
  +0x50 [dec 80]  x1  via EAX
  +0x74 [dec 116]  x1  via EDX
  +0x1D0 [dec 464]  x1  via EAX
  +0x3B4 [dec 948]  x1  via ECX
```

### Static facts (script output, report-only)
- size 108 B, RET 4
- callers 1: 0x008d9350
- callees: 0x004122a0 CSWGuiManager::PlayGuiSound (CALL), 0x008db170 (CALL), 0x008db3a0 (CALL)
- twins: none

## Card B17-4-15

GOG function: CSWGuiText::GetIdealHeight  gog 0x00915c10  callconv=(blank)  param_size_bytes=0  gog_size=95  gog_retn=0
Candidate Steam address: 0x004178b0 (size 91, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.83 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00417840:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00915bd0 CSWGuiText::wrapText  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00915c80 CSWGuiText::GetFontHeight -> 0x004178b0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x14*,0x48*,0x50} | Steam {0x14*,0x48*}
imm sets (* = in both, 1 common): GOG {0x10*} | Steam {0x10*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x004178b0: 16-byte aligned: YES
  004178b0  55             PUSH EBP
  004178b1  8B EC          MOV EBP,ESP
  004178b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004178b0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x14 [dec 20]  x7  via EAX,ECX
  +0x48 [dec 72]  x3  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 004178b0 FUN_004178b0 size=91 ret=0 callers=33 callees=0

int __fastcall FUN_004178b0(int param_1)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0x14) == 0) {
    iVar1 = 0;
  }
  else {
    iVar1 = (**(code **)(**(int **)(param_1 + 0x14) + 0x48))();
    if (iVar1 < 0x10) {
      iVar1 = (**(code **)(**(int **)(param_1 + 0x14) + 0x48))();
      iVar1 = iVar1 + 1;
    }
    else {
      iVar1 = (**(code **)(**(int **)(param_1 + 0x14) + 0x48))();
    }
  }
  return iVar1;
}
```

### decoy: 0x00417840 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00417840: 16-byte aligned: YES
  00417840  55             PUSH EBP
  00417841  8B EC          MOV EBP,ESP
  00417843  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00417840
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x14 [dec 20]  x7  via EAX,ECX
  +0x50 [dec 80]  x2  via EDX
  +0x48 [dec 72]  x1  via EDX
```

### Static facts (script output, report-only)
- size 91 B, RET 0
- callers 33: 0x0041ea50, 0x00753040, 0x0075bf60, 0x0075c080, 0x0075dd10, ...
- indirect: 0x004178d1 slot 0x48, 0x004178e9 slot 0x48, 0x00417903 slot 0x48
- twins: none

## Card B17-4-16

GOG function: CSWGuiLoadModuleDebugMenu::Constructor  gog 0x005b54b0  callconv=(blank)  param_size_bytes=4  gog_size=2532  gog_retn=4
Candidate Steam address: 0x008c0cb0 (size 2532, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 1 | A-CALLEE 5 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,CALLEE,SIG  (content level C2)
- evidence summary: STR ':modules'@009a5ad0; CALLEE 5 (callees: SubString 0x734a00, GetDirectoryList 0x736050, CenterPanel 0x40ea00; callers: Initialize 0x7be4c0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008c16a0:STR0/CALLEE0/SIG0.11/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005b5480 CSWGuiPowersFeatsSkillsDebugMenu::ReturnToMainAfterPick  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005b5ea0 CSWGuiCreateItemMenu::Destructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (34), mapped to Steam through trusted rows only:
  0x00407340 Global::GetOverridePath -> Steam ?  in candidate callees: n/a
  0x0044dd60 CExoArrayList_CExoString_::CExoArrayList_2 -> Steam ?  in candidate callees: n/a
  0x0044dda0 CExoArrayList_CExoString_::Destructor -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x005340d0 CExoArrayList_CSWGuiControl__::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x00534170 CExoArrayList::Add_2 -> Steam ?  in candidate callees: n/a
  0x005b60c0 CSWGuiLoadModuleDebugMenu::CreateOption -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  ... 26 more GOG callees
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x004ceb50 CGuiInGame::Initialize -> Steam 0x007be4c0  in candidate callers: yes
GOG strings (10): 'live%d'; 'MODULES:'; '_s.rim'; 'zzzzzzzz.zzz' ...
disp sets (* = in both, 4 common): GOG {0xc*,0x2cc*,0x344*,0x4a0*} | Steam {0xc*,0x2cc*,0x344*,0x4a0*}
imm sets (* = in both, 37 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+23)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+23)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x008c0cb0: 16-byte aligned: YES
  008c0cb0  55             PUSH EBP
  008c0cb1  8B EC          MOV EBP,ESP
  008c0cb3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x008c0cb0
string literals (12):
  0x009A8200  at 008c0d3a  'debug_p'
  0x009A18B8  at 008c0d60  'LB_OPTIONS'
  0x009A81F4  at 008c0da3  'LBL_BUILD'
  0x009A81EC  at 008c0df4  'Build: '
  0x00993E50  at 008c0ffb  'MODULES:'
  0x00993E50  at 008c103d  'MODULES:'
  0x0099FA68  at 008c10c0  'live%d'
  0x009A5AD0  at 008c1128  ':modules'
  0x009A5AC0  at 008c11fd  'zzzzzzzz.zzz'
  0x009A5AB8  at 008c1255  '_s.rim'
  0x009A5AC0  at 008c12c3  'zzzzzzzz.zzz'
  0x009A5AB8  at 008c131e  '_s.rim'
direct callees (34):
  0x0040E5A0  CSWGuiPanel::Constructor
  0x0041CE90  (unnamed)
  0x00419740  LabelCtor
  0x00710840  (unnamed)
  0x0040EE40  CSWGuiPanel::StartLoadFromLayout
  0x00733570  CExoString::CExoString_char
  0x0040F620  CSWGuiPanel::InitControl
  0x00733780  CExoString::~CExoString
  0x0040F5A0  CSWGuiPanel::StopLoadFromLayout
  0x00733DF0  CExoString::operator+
  0x00416E30  CSWGuiTextParams::SetText
  0x0084A640  (unnamed)
  0x00733540  CExoString::Constructor
  0x0040DEF0  (unnamed)
  0x00407800  (unnamed)
  0x00919671  (unnamed)
  0x00736050  CExoBase::GetDirectoryList
  0x00734270  CExoString::Format
  0x00739790  (unnamed)
  0x00919F10  _strlen
struct offsets touched (top 4):
  +0x2CC [dec 716]  x2  via ECX,EDX
  +0xC [dec 12]  x1  via EDX
  +0x344 [dec 836]  x1  via EDX
  +0x4A0 [dec 1184]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 008c0cb0 FUN_008c0cb0 size=2532 ret=4 callers=1 callees=34

void __thiscall FUN_008c0cb0(undefined4 *param_1,undefined4 param_2)

{
  uint uVar1;
  int *piVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  int iVar5;
  size_t local_21c;
  size_t local_218;
  size_t local_214;
  undefined1 local_20c [8];
  undefined1 local_204 [8];
  int local_1fc;
  undefined1 local_1f8 [8];
  undefined1 local_1f0 [40];
  undefined1 local_1c8 [8];
  undefined1 local_1c0 [8];
  undefined1 local_1b8 [8];
  undefined1 local_1b0 [8];
  undefined1 local_1a8 [8];
  undefined1 local_1a0 [16];
  undefined1 local_190 [8];
  undefined1 local_188 [8];
  undefined1 local_180 [16];
  int local_170;
  int local_16c;
  int local_168;
  int local_164;
  undefined1 local_160 [20];
  int local_14c;
  undefined1 local_148 [8];
  char local_140 [200];
  uint local_78;
  char *local_74 [5];
  int local_60;
  int local_5c;
  char *local_54 [2];
  undefined4 local_4c;
  int local_48;
  int local_44;
  undefined1 local_3c [8];
  undefined1 local_34 [12];
  undefined4 local_28;
  int local_24;
  int local_20;
  int local_1c;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0097a776;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_78 = uVar1;
  FUN_0040e5a0(param_2);
  local_8 = 0;
  *param_1 = CSWGuiLoadModuleDebugMenu::vftable;
  FUN_0041ce90(uVar1);
  local_8._0_1_ = 1;
  FUN_00419740();
  local_8._0_1_ = 2;
  param_1[0x128 = byte +0x4a0] = 0;
  FUN_00710840("debug_p");
  FUN_0040ee40(local_180,1,1);
  FUN_00733570("LB_OPTIONS");
  local_8._0_1_ = 3;
  FUN_0040f620(param_1 + 0x1a,local_188,1,1);
  local_8._0_1_ = 2;
  FUN_00733780();
  FUN_00733570("LBL_BUILD");
  local_8._0_1_ = 4;
  FUN_0040f620(param_1 + 0xd6,local_190,1,1);
  local_8._0_1_ = 2;
  FUN_00733780();
  FUN_0040f5a0();
  FUN_00733570("Build: ");
/* ... 200 more lines truncated by --max-lines */
```

### decoy: 0x008c16a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008c16a0: 16-byte aligned: YES
  008c16a0  55             PUSH EBP
  008c16a1  8B EC          MOV EBP,ESP
  008c16a3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008c16a0
string literals (0):
direct callees (3):
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x008C18E0  (unnamed)
  0x0040EAA0  CSWGuiPanel::HandleInputEvent
struct offsets touched (top 4):
  +0x4A0 [dec 1184]  x10  via ECX,EDX
  +0x1C [dec 28]  x4  via EAX,ECX
  +0x4A4 [dec 1188]  x3  via EAX,EDX
  +0x8C17D0 [dec 9181136]  x1  via EDX
```

### Static facts (script output, report-only)
- size 2532 B, RET 4
- callers 1: 0x007be4c0
- callees: 0x0040e5a0 CSWGuiPanel::Constructor (CALL), 0x0041ce90 (CALL), 0x00419740 (CALL), 0x00710840 (CALL), 0x0040ee40 CSWGuiPanel::StartLoadFromLayout (CALL), 0x00733570 CExoString::CStrConstructor (CALL), ...
- globals: 0x00a1b488 READ, 0x00a1b488 READ
- twins: none

## Card B17-4-17

GOG function: CSWGuiCreateItem::InitializeStoreType  gog 0x005c5330  callconv=(blank)  param_size_bytes=0  gog_size=108  gog_retn=4
Candidate Steam address: 0x008d4f60 (size 108, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008d4ac0:STR0/CALLEE0/SIG0.136/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x005c4e90 CSWGuiCreateItem::DeleteItemEntries  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005c53a0 CSWGuiCreateItem::OnControlEntered  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x005c69a0 CSWGuiCreateItem::ShowSellGUI -> Steam ?  in candidate callees: n/a
  0x005c6c20 CSWGuiCreateItem::ShowBuyGUI -> Steam ?  in candidate callees: n/a
  0x00910720 CSWGuiManager::PlayGuiSound -> Steam 0x004122a0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x1c*,0x3634*,0x3ed8*} | Steam {0x1c*,0x3634*,0x3ed8*}
imm sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 4)
```

### prologue
```
prologue for 0x008d4f60: 16-byte aligned: YES
  008d4f60  55             PUSH EBP
  008d4f61  8B EC          MOV EBP,ESP
  008d4f63  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008d4f60
string literals (0):
direct callees (3):
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x008D65D0  (unnamed)
  0x008D6850  (unnamed)
struct offsets touched (top 3):
  +0x3ED8 [dec 16088]  x3  via EAX,ECX,EDX
  +0x1C [dec 28]  x1  via EAX
  +0x3634 [dec 13876]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 008d4f60 FUN_008d4f60 size=108 ret=4 callers=1 callees=3

void __fastcall FUN_008d4f60(int param_1)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 0x3ed8) == 0) {
    *(undefined4 *)(param_1 + 0x3ed8) = 1;
  }
  else {
    *(undefined4 *)(param_1 + 0x3ed8) = 0;
  }
  uVar1 = *(undefined4 *)(param_1 + 0x1c);
  FUN_004122a0(0);
  if ((*(uint *)(param_1 + 0x3634) >> 1 & 1) == 0) {
    FUN_008d6850(param_1,uVar1);
  }
  else {
    FUN_008d65d0();
  }
  return;
}
```

### decoy: 0x008d4ac0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008d4ac0: 16-byte aligned: YES
  008d4ac0  55             PUSH EBP
  008d4ac1  8B EC          MOV EBP,ESP
  008d4ac3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x008d4ac0
string literals (0):
direct callees (21):
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x0073F750  CClientExoApp::GetInGameGui
  0x0075BB00  (unnamed)
  0x0075CA30  (unnamed)
  0x0075BBE0  (unnamed)
  0x0073FEA0  CClientExoApp::GetSWGuiManager
  0x00410530  CSWGuiManager::AddPanel
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x0051C0E0  CServerExoApp::GetItemByGameObjectID
  0x008D6B40  (unnamed)
  0x0051C8F0  CServerExoApp::GetPlayerCreatureId
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006A4E00  CSWSCreatureStats::GetLevel
  0x00733540  CExoString::Constructor
  0x00733700  CExoString::Constructor_5
  0x0071E910  (unnamed)
  0x00733780  CExoString::~CExoString
  0x00619C90  (unnamed)
  0x007337C0  CExoString::operator=
  0x007335F0  CExoString::CExoString_copy
struct offsets touched (top 10):
  +0x4 [dec 4]  x12  via EAX,ECX,EDX
  +0x8 [dec 8]  x9  via EAX,ECX,EDX
  +0xA0 [dec 160]  x3  via ECX,EDX
  +0x70 [dec 112]  x2  via EAX,EDX
  +0x1C [dec 28]  x1  via ECX
  +0x4C [dec 76]  x1  via EDX
  +0x50 [dec 80]  x1  via EAX
  +0x74 [dec 116]  x1  via EDX
  +0x1D0 [dec 464]  x1  via ECX
  +0x3B4 [dec 948]  x1  via ECX
```

### Static facts (script output, report-only)
- size 108 B, RET 4
- callers 1: 0x008d4260
- callees: 0x004122a0 CSWGuiManager::PlayGuiSound (CALL), 0x008d65d0 (CALL), 0x008d6850 (CALL)
- twins: none

## Card B17-4-18

GOG function: CSWGuiCreateItem::UpdateCreateFilters  gog 0x005c61f0  callconv=(blank)  param_size_bytes=4  gog_size=462  gog_retn=4
Candidate Steam address: 0x008d5e20 (size 462, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 4 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 4 (callees: Constructor 0x5ffd60, CopyItem 0x606600, SetComponent 0x5815d0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008d6090:STR0/CALLEE1/SIG0.236/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005c54b0 CSWGuiCreateItem::OnControlXButton -> 0x008d50e0  gate_class=AUTO
- GOG-order neighbour after:  0x005c63c0 CSWGuiCreateItem::SellItemCallback  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (14), mapped to Steam through trusted rows only:
  0x005c6f10 CSWGuiCreateItem::GetItemSellValue -> Steam ?  in candidate callees: n/a
  0x00605810 CExoString::Constructor_5 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x0064c310 CServerExoApp::GetPlayerCreatureId -> Steam 0x0051c8f0  in candidate callees: yes
  0x006634d0 CSWSItem::Constructor -> Steam 0x005ffd60  in candidate callees: yes
  0x00669e30 CSWSItem::CopyItem -> Steam 0x00606600  in candidate callees: yes
  0x0069a100 CSWSCreature::SetComponent -> Steam 0x005815d0  in candidate callees: yes
  ... 6 more GOG callees
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 7 common): GOG {0x4*,0x8*,0x1c*,0x389c*,0x38c2*,0x3ec4*,0x3edc*} | Steam {0x4*,0x8*,0x1c*,0x389c*,0x38c2*,0x3ec4*,0x3edc*}
imm sets (* = in both, 7 common): GOG {0x4*,0x9*,0x44*,0x30c*,0x3594*,0x35ec*,0x7f000000*} | Steam {0x4*,0x9*,0x44*,0x30c*,0x3594*,0x35ec*,0x7f000000*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x008d5e20: 16-byte aligned: YES
  008d5e20  55             PUSH EBP
  008d5e21  8B EC          MOV EBP,ESP
  008d5e23  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x008d5e20
string literals (0):
direct callees (14):
  0x0051C8F0  CServerExoApp::GetPlayerCreatureId
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x0058A2C0  CSWSCreature::GetItemRepository
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x008D6B40  (unnamed)
  0x005815D0  CSWSCreature::SetComponent
  0x00919723  operator new
  0x005FFD60  CSWSItem::Constructor
  0x00606600  CSWSItem::CopyItem
  0x005BDB60  CSWSCreature::AcquireItem
  0x0041E870  (unnamed)
  0x00733700  CExoString::Constructor_5
  0x00416E30  CSWGuiTextParams::SetText
  0x00733780  CExoString::~CExoString
struct offsets touched (top 7):
  +0x3EC4 [dec 16068]  x4  via EAX,ECX,EDX
  +0x389C [dec 14492]  x2  via EAX
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0x1C [dec 28]  x1  via EDX
  +0x38C2 [dec 14530]  x1  via ECX
  +0x3EDC [dec 16092]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 008d5e20 FUN_008d5e20 size=462 ret=4 callers=2 callees=14

void __thiscall FUN_008d5e20(int param_1,int param_2)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 local_4c;
  undefined4 local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0097ccf3;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  uVar1 = FUN_0051c8f0(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  iVar2 = FUN_0051c100(uVar1);
  if (((iVar2 != 0) && (local_18 = FUN_0058a2c0(1), local_18 != 0)) && (param_2 != 0)) {
    FUN_004122a0(9);
    iVar2 = FUN_008d6b40(param_2);
    *(int *)(param_1 + 0x3ec4) = *(int *)(param_1 + 0x3ec4) - iVar2;
    FUN_005815d0(*(undefined4 *)(param_1 + 0x3ec4));
    iVar2 = FUN_00919723(0x30c);
    local_8 = 0;
    if (iVar2 == 0) {
      local_4c = 0;
    }
    else {
      local_4c = FUN_005ffd60(0x7f000000);
    }
    local_8 = 0xffffffff;
    local_1c = local_4c;
    FUN_00606600(param_2);
    FUN_005bdb60(&local_1c,*(undefined4 *)(*(int *)(param_1 + 0x3edc) + 4),0x7f000000,1);
    local_14 = (int)*(short *)(param_1 + 0x38c2);
    if (*(int *)(param_1 + 0x389c) <= local_14) {
      local_14 = *(int *)(param_1 + 0x389c) + -1;
    }
    FUN_0041e870(local_14,0);
    uVar1 = FUN_00733700(*(undefined4 *)(param_1 + 0x3ec4));
    local_8 = 1;
    FUN_00416e30(uVar1);
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x008d6090 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008d6090: 16-byte aligned: YES
  008d6090  55             PUSH EBP
  008d6091  8B EC          MOV EBP,ESP
  008d6093  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x008d6090
string literals (0):
direct callees (14):
  0x0051C8F0  CServerExoApp::GetPlayerCreatureId
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x0058A2C0  CSWSCreature::GetItemRepository
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x008D6B40  (unnamed)
  0x0091C860  _ftol2_sse
  0x005815D0  CSWSCreature::SetComponent
  0x00604EC0  CSWSItem::SplitItem
  0x00605E80  CItemRepository::RemoveItem
  0x008D2FF0  (unnamed)
  0x0041E870  (unnamed)
  0x00733700  CExoString::Constructor_5
  0x00416E30  CSWGuiTextParams::SetText
  0x00733780  CExoString::~CExoString
struct offsets touched (top 7):
  +0x3EC4 [dec 16068]  x4  via ECX,EDX
  +0x3B8C [dec 15244]  x2  via EAX
  +0x3ED4 [dec 16084]  x2  via ECX,EDX
  +0x8 [dec 8]  x1  via EAX
  +0x1C [dec 28]  x1  via EDX
  +0x2CC [dec 716]  x1  via ECX
  +0x3BB2 [dec 15282]  x1  via ECX
```

### Static facts (script output, report-only)
- size 462 B, RET 4
- callers 2: 0x008d4ac0, 0x008d5ff0
- callees: 0x0051c8f0 CServerExoApp::GetPlayerCreatureId (CALL), 0x0051c100 CServerExoApp::GetCreatureByGameObjectID (CALL), 0x0058a2c0 CSWSCreature::GetItemRepository (CALL), 0x004122a0 CSWGuiManager::PlayGuiSound (CALL), 0x008d6b40 (CALL), 0x005815d0 CSWSCreature::SetComponent (CALL), ...
- twins: none

## Card B17-4-19

GOG function: CSWGuiContainer::OnPanelAdded  gog 0x005a71d0  callconv=(blank)  param_size_bytes=0  gog_size=243  gog_retn=0
Candidate Steam address: 0x008b3630 (size 243, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callees: OnPanelAdded 0x40f3c0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008b35d0:STR0/CALLEE0/SIG0.083/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005a7170 CSWGuiContainer::Update -> 0x008b35d0  gate_class=AUTO
- GOG-order neighbour after:  0x005a72d0 CSWGuiContainer::OnPanelRemoved  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0040c380 Global::IsJoystickConnected -> Steam ?  in candidate callees: n/a
  0x0090d840 CSWGuiPanel::OnPanelAdded -> Steam 0x0040f3c0  in candidate callees: yes
  0x009125d0 CSWGuiManager::StoreCurrentMousePosition -> Steam ?  in candidate callees: n/a
  0x009126b0 CSWGuiManager::MoveMouseToPosition -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0xc*} | Steam {0x4*,0x8*,0xc*}
imm sets (* = in both, 3 common): GOG {0x4*,0x2c*,0xb18*} | Steam {0x4*,0x2c*,0xb18*}

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

### decoy: 0x008b35d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008b35d0: 16-byte aligned: YES
  008b35d0  55             PUSH EBP
  008b35d1  8B EC          MOV EBP,ESP
  008b35d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008b35d0
string literals (0):
direct callees (1):
  0x008B2F70  CSWGuiContainer::SetupGiveMode
struct offsets touched (top 1):
  +0x70 [dec 112]  x7  via EAX,ECX,EDX
```

### Static facts (script output, report-only)
- size 243 B, RET 0
- callees: 0x0073eee0 (CALL), 0x00414150 (CALL), 0x00414230 (CALL), 0x0040f3c0 CSWGuiPanel::OnPanelAdded (CALL)
- globals: 0x00a1b49c READ
- twins: none

## Card B17-4-20

GOG function: Global::polyhit_2  gog 0x0080e4d0  callconv=(blank)  param_size_bytes=24  gog_size=1874  gog_retn=0
Candidate Steam address: 0x00516da0 (size 1323, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.65 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00516640:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0080e0c0 Global::polyhit -> 0x00516230  gate_class=REVIEW
- GOG-order neighbour after:  0x0080ec30 Global::BoxIntersect -> 0x00516da0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0080c0a0 Global::float_magnitude_Vector_const_ref -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 5): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
imm sets (* = in both, 4 common): GOG {0x2*,0x4*,0x5*,0xc,0x41*,0x44,0x18c} | Steam {0x2*,0x3,0x4*,0x5*,0x6,0x8,0x10,0x20,0x41*,0x4c}

### abi
```
GOG param bytes: 24; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(24/4)*4 = 24 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 24 bytes -> x86 slots 24; ASM RET 0)
```

### prologue
```
prologue for 0x00516da0: 16-byte aligned: YES
  00516da0  55             PUSH EBP
  00516da1  8B EC          MOV EBP,ESP
  00516da3  83 EC 4C       SUB ESP,0x4c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00516da0
string literals (0):
direct callees (1):
  0x005172D0  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x32  via EAX,ECX,EDX
  +0x8 [dec 8]  x32  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00516da0 FUN_00516da0 size=1323 ret=0 callers=1 callees=1

undefined4 FUN_00516da0(float *param_1,float *param_2,float *param_3,float *param_4,float *param_5)

{
  float fVar1;
  float fVar2;
  undefined4 uVar3;
  byte local_6;
  byte local_5;
  
  FUN_005172d0(param_1,param_3,param_4,&local_6);
  if (local_6 == 0) {
    uVar3 = 0xffffffff;
  }
  else {
    FUN_005172d0(param_2,param_3,param_4,&local_5);
    if ((local_6 & local_5) == 0) {
      if ((local_6 & 0x20) == 0) {
        if ((local_6 & 0x10) != 0) {
          fVar1 = (*param_4 - *param_1) / (*param_2 - *param_1);
          fVar2 = fVar1 * param_2[1 = byte +0x4] + (1.0 - fVar1) * param_1[1 = byte +0x4];
          fVar1 = fVar1 * param_2[2 = byte +0x8] + (1.0 - fVar1) * param_1[2 = byte +0x8];
          if ((((param_3[1 = byte +0x4] < fVar2 != (param_3[1 = byte +0x4] == fVar2)) && (fVar2 <= param_4[1 = byte +0x4])) &&
              (param_3[2 = byte +0x8] < fVar1 != (param_3[2 = byte +0x8] == fVar1))) && (fVar1 <= param_4[2 = byte +0x8])) {
            *param_5 = *param_4;
            param_5[1 = byte +0x4] = fVar2;
            param_5[2 = byte +0x8] = fVar1;
            return 2;
          }
        }
      }
      else {
        fVar1 = (*param_3 - *param_1) / (*param_2 - *param_1);
        fVar2 = fVar1 * param_2[1 = byte +0x4] + (1.0 - fVar1) * param_1[1 = byte +0x4];
        fVar1 = fVar1 * param_2[2 = byte +0x8] + (1.0 - fVar1) * param_1[2 = byte +0x8];
        if (((param_3[1 = byte +0x4] < fVar2 != (param_3[1 = byte +0x4] == fVar2)) && (fVar2 <= param_4[1 = byte +0x4])) &&
           ((param_3[2 = byte +0x8] < fVar1 != (param_3[2 = byte +0x8] == fVar1) && (fVar1 <= param_4[2 = byte +0x8])))) {
          *param_5 = *param_3;
          param_5[1 = byte +0x4] = fVar2;
          param_5[2 = byte +0x8] = fVar1;
          return 1;
        }
      }
      if ((local_6 & 8) == 0) {
        if ((local_6 & 4) != 0) {
          fVar1 = (param_4[1 = byte +0x4] - param_1[1 = byte +0x4]) / (param_2[1 = byte +0x4] - param_1[1 = byte +0x4]);
          fVar2 = fVar1 * *param_2 + (1.0 - fVar1) * *param_1;
          fVar1 = fVar1 * param_2[2 = byte +0x8] + (1.0 - fVar1) * param_1[2 = byte +0x8];
          if (((*param_3 < fVar2 != (*param_3 == fVar2)) && (fVar2 <= *param_4)) &&
             ((param_3[2 = byte +0x8] < fVar1 != (param_3[2 = byte +0x8] == fVar1) && (fVar1 <= param_4[2 = byte +0x8])))) {
            *param_5 = fVar2;
            param_5[1 = byte +0x4] = param_4[1 = byte +0x4];
            param_5[2 = byte +0x8] = fVar1;
            return 4;
          }
        }
      }
      else {
        fVar1 = (param_3[1 = byte +0x4] - param_1[1 = byte +0x4]) / (param_2[1 = byte +0x4] - param_1[1 = byte +0x4]);
        fVar2 = fVar1 * *param_2 + (1.0 - fVar1) * *param_1;
        fVar1 = fVar1 * param_2[2 = byte +0x8] + (1.0 - fVar1) * param_1[2 = byte +0x8];
        if ((((*param_3 < fVar2 != (*param_3 == fVar2)) && (fVar2 <= *param_4)) &&
            (param_3[2 = byte +0x8] < fVar1 != (param_3[2 = byte +0x8] == fVar1))) && (fVar1 <= param_4[2 = byte +0x8])) {
          *param_5 = fVar2;
          param_5[1 = byte +0x4] = param_3[1 = byte +0x4];
          param_5[2 = byte +0x8] = fVar1;
          return 3;
        }
      }
      if ((local_6 & 2) == 0) {
        if ((local_6 & 1) != 0) {
          fVar1 = (param_4[2 = byte +0x8] - param_1[2 = byte +0x8]) / (param_2[2 = byte +0x8] - param_1[2 = byte +0x8]);
          fVar2 = fVar1 * *param_2 + (1.0 - fVar1) * *param_1;
          fVar1 = fVar1 * param_2[1 = byte +0x4] + (1.0 - fVar1) * param_1[1 = byte +0x4];
          if (((param_3[1 = byte +0x4] < fVar1 != (param_3[1 = byte +0x4] == fVar1)) && (fVar1 <= param_4[1 = byte +0x4])) &&
             ((*param_3 < fVar2 != (*param_3 == fVar2) && (fVar2 <= *param_4)))) {
            *param_5 = fVar2;
            param_5[1 = byte +0x4] = fVar1;
            param_5[2 = byte +0x8] = param_4[2 = byte +0x8];
            return 6;
/* ... 23 more lines truncated by --max-lines */
```

### decoy: 0x00516640 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00516640: 16-byte aligned: YES
  00516640  55             PUSH EBP
  00516641  8B EC          MOV EBP,ESP
  00516643  81 EC 8C 01 00 00 SUB ESP,0x18c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00516640
string literals (0):
direct callees (1):
  0x005142F0  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x13  via EAX,ECX,EDX
  +0x8 [dec 8]  x13  via EAX,ECX,EDX
```

### Static facts (script output, report-only)
- size 1323 B, RET 0
- callers 1: 0x0045d620
- callees: 0x005172d0 (CALL)
- twins: none

## Card B17-4-21

GOG function: Global::CullUnnecessaryKeys  gog 0x008c68e0  callconv=(blank)  param_size_bytes=4  gog_size=874  gog_retn=0
Candidate Steam address: 0x00486700 (size 505, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.42 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00486390:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x008c6720 Global::AddGenericControllerSingle  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008c6c50 Global::AddGenericController  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x008cebd0 unnamed -> Steam ?  in candidate callees: n/a
  0x00925c49 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0xc*,0x10} | Steam {0x4*,0x8*,0xc*,0x28,0x50}
imm sets (* = in both, 3 common): GOG {0x2*,0x8*,0x2c*,0x41,0xa4} | Steam {0x2*,0x3,0x4,0x8*,0x10,0x2c*,0x200}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x00486700: 16-byte aligned: YES
  00486700  55             PUSH EBP
  00486701  8B EC          MOV EBP,ESP
  00486703  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00486700
string literals (0):
direct callees (7):
  0x0048E5C0  (unnamed)
  0x0048E740  (unnamed)
  0x00463B20  (unnamed)
  0x00486390  (unnamed)
  0x00485D50  (unnamed)
  0x00451660  (unnamed)
  0x0048E460  (unnamed)
struct offsets touched (top 4):
  +0x4 [dec 4]  x3  via EAX,ECX
  +0x8 [dec 8]  x1  via EAX
  +0xC [dec 12]  x1  via EAX
  +0x50 [dec 80]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00486700 FUN_00486700 size=505 ret=0 callers=4 callees=7

void FUN_00486700(undefined4 param_1,undefined4 param_2,undefined4 param_3,int param_4,
                 undefined4 param_5,undefined2 param_6)

{
  undefined2 uVar1;
  uint uVar2;
  int local_38;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  int local_24;
  int local_20;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094af98;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_0048e5c0(0);
  local_8 = 0;
  FUN_0048e740(param_2,&local_24,uVar2);
  local_30 = CONCAT22((undefined2)local_20,param_6);
  if (local_20 == 0) {
    local_8 = 0xffffffff;
    FUN_00463b20();
  }
  else {
    local_14 = *(int *)(local_24 + 0x28) + -1;
    if (local_14 == 1) {
      local_28 = 1;
    }
    else if (local_14 == 3) {
      local_28 = 3;
    }
    else if (local_14 == 4) {
      local_28 = 4;
      if (DAT_009f6150 != 0) {
        FUN_00486390(&local_24);
        local_30 = CONCAT22((undefined2)local_20,param_6);
      }
      if ((DAT_009f6148 != 0) && ((*(ushort *)(DAT_00a32e04 + 0x50) & 0x200) == 0)) {
        FUN_00485d50(&local_24);
        local_28 = 2;
      }
    }
    uVar1 = *(undefined2 *)(param_4 + 4);
    for (local_18 = 0; local_18 < local_20; local_18 = local_18 + 1) {
      FUN_00451660(*(undefined4 *)(local_24 + local_18 * 0x2c));
    }
    local_2c = CONCAT22(*(undefined2 *)(param_4 + 4),uVar1);
    for (local_18 = 0; local_18 < local_20; local_18 = local_18 + 1) {
      for (local_38 = 1; local_38 < *(int *)(local_24 + 0x28 + local_18 * 0x2c);
          local_38 = local_38 + 1) {
        FUN_00451660(*(undefined4 *)(local_18 * 0x2c + local_24 + local_38 * 4));
      }
    }
    DAT_00a32e08 = DAT_00a32e08 + local_20;
    FUN_0048e460(param_5,local_30,local_2c,local_28);
    local_8 = 0xffffffff;
    FUN_00463b20();
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x00486390 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00486390: 16-byte aligned: YES
  00486390  55             PUSH EBP
  00486391  8B EC          MOV EBP,ESP
  00486393  81 EC A4 00 00 00 SUB ESP,0xa4
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00486390
string literals (0):
direct callees (2):
  0x0091EBAD  (unnamed)
  0x0048E680  (unnamed)
struct offsets touched (top 4):
  +0x4 [dec 4]  x5  via EAX,ECX,EDX
  +0x8 [dec 8]  x4  via ECX,EDX
  +0xC [dec 12]  x4  via ECX,EDX
  +0x10 [dec 16]  x4  via ECX,EDX
```

### Static facts (script output, report-only)
- size 505 B, RET 0
- callers 4: 0x00486aa0, 0x00486fb0, 0x004878b0, 0x0048b6f0
- callees: 0x0048e5c0 (CALL), 0x0048e740 (CALL), 0x00463b20 (CALL), 0x00486390 (CALL), 0x00485d50 (CALL), 0x00451660 (CALL), ...
- twins: none

## Card B17-4-22

GOG function: Global::polyhit_2  gog 0x0080e4d0  callconv=(blank)  param_size_bytes=24  gog_size=1874  gog_retn=0
Candidate Steam address: 0x00516230 (size 1033, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.81 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00516640:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0080e0c0 Global::polyhit -> 0x00516230  gate_class=REVIEW
- GOG-order neighbour after:  0x0080ec30 Global::BoxIntersect -> 0x00516da0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0080c0a0 Global::float_magnitude_Vector_const_ref -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 5): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
imm sets (* = in both, 5 common): GOG {0x2*,0x4,0x5*,0xc*,0x41*,0x44*,0x18c} | Steam {0x2*,0x5*,0xc*,0x41*,0x44*,0xc4}

### abi
```
GOG param bytes: 24; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(24/4)*4 = 24 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 24 bytes -> x86 slots 24; ASM RET 0)
```

### prologue
```
prologue for 0x00516230: 16-byte aligned: YES
  00516230  55             PUSH EBP
  00516231  8B EC          MOV EBP,ESP
  00516233  81 EC C4 00 00 00 SUB ESP,0xc4
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00516230
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x11  via EAX,ECX,EDX
  +0x8 [dec 8]  x11  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00516230 FUN_00516230 size=1033 ret=0 callers=2 callees=0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool FUN_00516230(int param_1,int param_2,float *param_3,float *param_4,float *param_5,
                 float *param_6,float *param_7)

{
  float fVar1;
  float fVar2;
  bool bVar3;
  float *pfVar4;
  float *pfVar5;
  float *pfVar6;
  int local_18;
  
  fVar1 = *param_3 * *param_5 + param_3[1 = byte +0x4] * param_5[1 = byte +0x4] + param_3[2 = byte +0x8] * param_5[2 = byte +0x8] + *param_4;
  fVar2 = *param_3 * *param_6 + param_3[1 = byte +0x4] * param_6[1 = byte +0x4] + param_3[2 = byte +0x8] * param_6[2 = byte +0x8] + *param_4;
  if ((fVar1 < (float)_DAT_00986b38) || ((float)_DAT_00986b38 < fVar2)) {
    bVar3 = false;
  }
  else if (fVar2 == fVar1) {
    bVar3 = false;
  }
  else {
    if ((DAT_00a7d824 & 1) == 0) {
      DAT_00a7d824 = DAT_00a7d824 | 1;
    }
    if ((DAT_00a7d824 & 2) == 0) {
      DAT_00a7d824 = DAT_00a7d824 | 2;
      _DAT_00a7d80c = 0.0;
      _DAT_00a7d810 = 0.0;
      _DAT_00a7d814 = 0.0;
    }
    fVar1 = fVar1 / (fVar1 - fVar2);
    fVar2 = 1.0 - fVar1;
    DAT_00a7d818 = fVar1 * *param_6 + fVar2 * *param_5;
    DAT_00a7d81c = fVar1 * param_6[1 = byte +0x4] + fVar2 * param_5[1 = byte +0x4];
    DAT_00a7d820 = fVar1 * param_6[2 = byte +0x8] + fVar2 * param_5[2 = byte +0x8];
    bVar3 = true;
    for (local_18 = 0; (bVar3 != false && (local_18 < param_2)); local_18 = local_18 + 1) {
      pfVar5 = (float *)(local_18 * 0xc + param_1);
      pfVar4 = (float *)(local_18 * 0xc + param_1);
      pfVar6 = (float *)(((local_18 + 1) % param_2) * 0xc + param_1);
      _DAT_00a7d80c =
           (DAT_00a7d820 - pfVar5[2 = byte +0x8]) * (pfVar6[1 = byte +0x4] - pfVar4[1 = byte +0x4]) -
           (DAT_00a7d81c - pfVar5[1 = byte +0x4]) * (pfVar6[2 = byte +0x8] - pfVar4[2 = byte +0x8]);
      _DAT_00a7d810 =
           (DAT_00a7d818 - *pfVar5) * (pfVar6[2 = byte +0x8] - pfVar4[2 = byte +0x8]) -
           (DAT_00a7d820 - pfVar5[2 = byte +0x8]) * (*pfVar6 - *pfVar4);
      _DAT_00a7d814 =
           (DAT_00a7d81c - pfVar5[1 = byte +0x4]) * (*pfVar6 - *pfVar4) -
           (DAT_00a7d818 - *pfVar5) * (pfVar6[1 = byte +0x4] - pfVar4[1 = byte +0x4]);
      bVar3 = (float)_DAT_00992300 <=
              _DAT_00a7d80c * *param_3 + _DAT_00a7d810 * param_3[1 = byte +0x4] + _DAT_00a7d814 * param_3[2 = byte +0x8];
    }
    if ((bVar3 != false) && (param_7 != (float *)0x0)) {
      *param_7 = DAT_00a7d818;
      param_7[1 = byte +0x4] = DAT_00a7d81c;
      param_7[2 = byte +0x8] = DAT_00a7d820;
    }
  }
  return bVar3;
}
```

### decoy: 0x00516640 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00516640: 16-byte aligned: YES
  00516640  55             PUSH EBP
  00516641  8B EC          MOV EBP,ESP
  00516643  81 EC 8C 01 00 00 SUB ESP,0x18c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00516640
string literals (0):
direct callees (1):
  0x005142F0  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x13  via EAX,ECX,EDX
  +0x8 [dec 8]  x13  via EAX,ECX,EDX
```

### Static facts (script output, report-only)
- size 1033 B, RET 0
- callers 2: 0x0045d620, 0x0045dfa0
- twins: none
