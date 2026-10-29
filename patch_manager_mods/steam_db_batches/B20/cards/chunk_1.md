# Review cards B20 chunk 1

Answer each card with one TSV row in `steam_db_batches/B20/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B20-1-01

GOG function: CResRef::operator==_3  gog 0x0061a570  callconv=(blank)  param_size_bytes=4  gog_size=113  gog_retn=4
Candidate Steam address: 0x00710b70 (size 164, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00710c20:STR0/CALLEE10/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061a4c0 CResRef::operator==_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0061a5f0 CResRef::operator=  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00928a62 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (10 of 40): the card's A-CALLEE credit may come from these, not from callees
  0x00411610 CSWCCreature::SetSoundSet -> Steam 0x00765940  in candidate callers: no
  0x00424b60 CSWCCreature::GetEquipmentForcePowers_2 -> Steam 0x00778ba0  in candidate callers: no
  0x00475550 CSWCObject::SetEnvMap -> Steam 0x007f03f0  in candidate callers: no
  0x004756d0 CSWCObject::SetEnvMap_2 -> Steam 0x007f0570  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x4*,0x10*} | Steam {0x4*,0xc,0x10*,0x18}

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

### decoy: 0x00710c20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00710c20: 16-byte aligned: YES
  00710c20  55             PUSH EBP
  00710c21  8B EC          MOV EBP,ESP
  00710c23  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00710c20
string literals (0):
direct callees (1):
  0x0091E01A  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 164 B, RET 4
- callers 1: 0x00710aa0
- callees: 0x00734010 (CALL), 0x00919f10 _strlen (CALL), 0x0091ac7c (CALL)
- twins: none

## Card B20-1-02

GOG function: CSWCVisualEffectOnObject::ApplyLightsaberThrow  gog 0x004fec40  callconv=(blank)  param_size_bytes=0  gog_size=1948  gog_retn=4
Candidate Steam address: 0x00873360 (size 1948, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.95) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callees: Constructor 0x8e54e0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00873070:STR0/CALLEE0/SIG0.34/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x004fe950 CSWCVisualEffectOnObject::ApplySpellVisual  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004ff3e0 CSWCVisualEffectOnObject::ApplyTextureReplace  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (22), mapped to Steam through trusted rows only:
  0x0040c8f0 CClientExoApp::GetModule -> Steam 0x0073f430  in candidate callees: yes
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
disp sets (* = in both, 13 common): GOG {0x4*,0x8*,0x2c*,0x48*,0x78*,0x88*,0x8c*,0x94*,0x98*,0xe0*,0x158*,0x188*,0x1c4*} | Steam {0x4*,0x8*,0x2c*,0x48*,0x78*,0x88*,0x8c*,0x94*,0x98*,0xe0*,0x158*,0x188*,0x1c4*}
imm sets (* = in both, 14 common): GOG {0x2*,0x3*,0x4*,0xc*,0x10*,0x24*,0x30*,0x31*,0x44*,0xff*,0x114*,0x128*,0x148*,0x1c8*} | Steam {0x2*,0x3*,0x4*,0xc*,0x10*,0x24*,0x30*,0x31*,0x44*,0xff*,0x114*,0x128*,0x148*,0x1c8*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 4)
```

### prologue
```
prologue for 0x00873360: 16-byte aligned: YES
  00873360  55             PUSH EBP
  00873361  8B EC          MOV EBP,ESP
  00873363  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 00873360 FUN_00873360 size=1948 ret=4 callers=1 callees=22

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00873360(void)

{
  int iVar1;
  undefined4 uVar2;
  int *piVar3;
  float10 fVar4;
  int *local_14c;
  float *local_120;
  float local_cc;
  float local_c8;
  float local_c4;
  undefined1 local_c0 [8];
  undefined1 local_b8 [8];
  float local_b0;
  float local_ac;
  float local_a8;
  undefined4 local_a4;
  undefined4 local_a0;
  undefined4 local_9c;
  undefined1 local_98 [16];
  int local_88;
  int *local_84;
  int local_80;
  undefined4 local_7c;
  undefined4 local_78;
  undefined4 local_74;
  undefined4 local_70;
  int *local_6c;
  float local_68;
  float local_64;
  float local_60;
  float local_5c;
  undefined4 local_58;
  float local_54;
  float local_50;
  float local_4c;
  float local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  int local_38;
  float local_34;
  float local_30;
  float local_2c;
  int *local_28;
  int local_24;
  int *local_20;
  int *local_1c;
  float local_18;
  int *local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00973f24;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_20 = (int *)FUN_00877dc0(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_14 = (int *)FUN_00877e10();
  local_1c = (int *)FUN_00877e60();
  local_18 = _DAT_00994f50;
  if (local_1c != (int *)0x0) {
    iVar1 = (**(code **)(*local_1c + 0x2c))();
    if (iVar1 != 0) {
      (**(code **)(*local_1c + 0x2c))();
      fVar4 = (float10)FUN_0077fce0();
      local_18 = (float)fVar4;
    }
  }
  if (((local_20 != (int *)0x0) && (local_14 != (int *)0x0)) && (local_1c != (int *)0x0)) {
    iVar1 = (**(code **)(*local_1c + 0x2c))();
    if (iVar1 != 0) {
      local_28 = (int *)(**(code **)(*local_1c + 0x2c))();
      uVar2 = FUN_0076e610(0x10);
      local_24 = FUN_0073f530(uVar2);
/* ... 105 more lines truncated by --max-lines */
```

### decoy: 0x00873070 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00873070: 16-byte aligned: YES
  00873070  55             PUSH EBP
  00873071  8B EC          MOV EBP,ESP
  00873073  81 EC 80 00 00 00 SUB ESP,0x80
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

### Static facts (script output, report-only)
- size 1948 B, RET 4
- callers 1: 0x00872910
- callees: 0x00877dc0 (CALL), 0x00877e10 (CALL), 0x00877e60 (CALL), 0x0077fce0 (CALL), 0x0076e610 (CALL), 0x0073f530 CClientExoApp::GetItemByGameObjectID (CALL), ...
- indirect: 0x008733d2 slot 0x2c, 0x008733e3 slot 0x2c, 0x00873418 slot 0x2c, 0x0087342d slot 0x2c, 0x00873512 slot 0x78, ...
- twins: none

## Card B20-1-03

GOG function: CGuiInGame::Constructor  gog 0x004cd090  callconv=(blank)  param_size_bytes=0  gog_size=3333  gog_retn=0
Candidate Steam address: 0x007bce00 (size 3333, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 2 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,CALLEE,SIG  (content level C2)
- evidence summary: STR 'Display Options'@009a04bc; 'Compass'@009a04cc; CALLEE 2 (callees: SetCameraMode 0x7b5e50; callers: StartServices 0x788d90); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007bdb10:STR0/CALLEE0/SIG0.056/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004cce20 CSWCAnimBase::SetGroundTilt  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004cdda0 CGuiInGame::ShutDownGlobal  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (18), mapped to Steam through trusted rows only:
  0x0040ccf0 CClientExoApp::GetClientOptions -> Steam 0x0072fb00  in candidate callees: yes
  0x00462bc0 CClientOptions::SetCameraMode -> Steam 0x007b5e50  in candidate callees: yes
  0x00495270 unnamed -> Steam ?  in candidate callees: n/a
  0x004e29e0 CSWCMessage::HandleServerToPlayerUpdate_Appearance -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00567c10 CExoArrayList_int_::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  ... 10 more GOG callees
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x004353b0 CClientExoAppInternal::StartServices -> Steam 0x00788d90  in candidate callers: yes
GOG strings (3): '.\\\\swkotor2.ini'; 'Display Options'; 'Compass'
disp sets (* = in both, 137 common): GOG {0x4*,0x8*,0xc*,0x2c*,0x30*,0x34*,0x38*,0x3c*,0x40*,0x44*,0x48*,0x4c*,0x50*,0x54*,...(+123)} | Steam {0x4*,0x8*,0xc*,0x2c*,0x30*,0x34*,0x38*,0x3c*,0x40*,0x44*,0x48*,0x4c*,0x50*,0x54*,...(+123)}
imm sets (* = in both, 36 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+22)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+22)}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007bce00: 16-byte aligned: YES
  007bce00  55             PUSH EBP
  007bce01  8B EC          MOV EBP,ESP
  007bce03  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007bce00
string literals (3):
  0x009A04CC  at 007bd728  'Compass'
  0x009A04BC  at 007bd739  'Display Options'
  0x00986B98  at 007bd74a  '.\\\\swkotor2.ini'
direct callees (18):
  0x00733540  CExoString::Constructor
  0x00722870  (unnamed)
  0x0091D67E  (unnamed)
  0x007107F0  (unnamed)
  0x007D22E0  (unnamed)
  0x005933B0  (unnamed)
  0x0084A640  (unnamed)
  0x0072FB00  CClientExoApp::GetClientOptions
  0x007B5E50  CClientOptions::SetCameraMode
  0x00919723  operator new
  0x007338D0  CExoString::operator=_2
  0x00736240  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x00736320  (unnamed)
  0x00733780  CExoString::~CExoString
  0x0091CF18  (unnamed)
  0x00710EB0  CResRef::operator=_4
  0x007362C0  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x6  via EAX,ECX,EDX
  +0x8 [dec 8]  x6  via EAX,EDX
  +0xFC4 [dec 4036]  x2  via EAX,EDX
  +0xC [dec 12]  x1  via EDX
  +0x2C [dec 44]  x1  via ECX
  +0x30 [dec 48]  x1  via EAX
  +0x34 [dec 52]  x1  via EDX
  +0x38 [dec 56]  x1  via EAX
  +0x3C [dec 60]  x1  via EAX
  +0x40 [dec 64]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007bce00 FUN_007bce00 size=3333 ret=0 callers=1 callees=18

uint * __fastcall FUN_007bce00(uint *param_1)

{
  uint uVar1;
  undefined4 uVar2;
  undefined4 *local_a0;
  undefined4 *local_9c;
  undefined4 *local_98;
  int local_90;
  int local_84;
  uint *local_80;
  undefined1 local_64 [16];
  undefined1 local_54 [8];
  undefined1 local_4c [8];
  undefined1 local_44 [8];
  int local_3c;
  undefined4 *local_38;
  undefined4 *local_34;
  undefined4 *local_30;
  undefined4 *local_2c;
  undefined4 *local_28;
  undefined4 *local_24;
  byte local_19;
  int local_18[2 = byte +0x8];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0096965b;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  param_1[0x40 = byte +0x100] = 0;
  param_1[0x41 = byte +0x104] = 0;
  param_1[0x42 = byte +0x108] = 0;
  local_84 = 0x46;
  local_80 = param_1 + 0x75;
  while (local_84 = local_84 + -1, -1 < local_84) {
    local_80[1 = byte +0x4] = 0;
    local_80[2 = byte +0x8] = 0;
    local_80[3 = byte +0xc] = 0;
    local_80[7 = byte +0x1c] = 0;
    local_80[6 = byte +0x18] = 0;
    local_80[5 = byte +0x14] = 0;
    local_80[4 = byte +0x10] = 0x3f800000;
    local_80 = local_80 + 0xc;
  }
  FUN_00733540(uVar1);
  local_8 = 0;
  FUN_00722870();
  local_8._0_1_ = 1;
  FUN_00722870();
  local_8._0_1_ = 2;
  FUN_00722870();
  local_8._0_1_ = 3;
  _eh_vector_constructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable](param_1 + 0x3ce,8,8,FUN_00733540,FUN_00733780);
  local_8._0_1_ = 4;
  FUN_007107f0();
  param_1[0x403 = byte +0x100c] = 0;
  param_1[0x404 = byte +0x1010] = 0;
  param_1[0x405 = byte +0x1014] = 0;
  FUN_007d22e0(0);
  local_8._0_1_ = 5;
  FUN_005933b0(0);
  local_8._0_1_ = 6;
  FUN_0084a640(0);
  local_8._0_1_ = 7;
  uVar2 = 3;
  FUN_0072fb00(3);
  FUN_007b5e50(uVar2);
  local_28 = (undefined4 *)FUN_00919723(0x404);
  local_8._0_1_ = 8;
  if (local_28 == (undefined4 *)0x0) {
    local_98 = (undefined4 *)0x0;
  }
  else {
    *local_28 = 0x40;
    _eh_vector_constructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable](local_28 + 0x1 [dec 1] {undefined4*: byte +0x4},0x10,0x40,FUN_008fe650,FUN_006c5370);
/* ... 212 more lines truncated by --max-lines */
```

### decoy: 0x007bdb10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007bdb10: 16-byte aligned: YES
  007bdb10  55             PUSH EBP
  007bdb11  8B EC          MOV EBP,ESP
  007bdb13  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007bdb10
string literals (0):
direct callees (3):
  0x00919BCC  `eh vector destructor iterator` (CRT)
  0x009196FD  operator delete
  0x00733780  CExoString::~CExoString
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 3333 B, RET 0
- callers 1: 0x00788d90 CClientExoAppInternal::StartServices
- callees: 0x00733540 CExoString::DefaultConstructor (CALL), 0x00722870 (CALL), 0x0091d67e (CALL), 0x007107f0 (CALL), 0x007d22e0 (CALL), 0x005933b0 (CALL), ...
- strings: 0x009a04bc "Display Options", 0x009a04cc "Compass"
- globals: 0x00a1b4a4 READ
- twins: none

## Card B20-1-04

GOG function: CExoInput::UpdateMouseDelta  gog 0x00616990  callconv=(blank)  param_size_bytes=4  gog_size=31  gog_retn=4
Candidate Steam address: 0x0072fd10 (size 81, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.42 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0072fd70:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00616970 CExoInput::GetMouseDelta  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x006169b0 CExoInput::GetJoystickNStick  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00615ec0 CExoInputInternal::UpdateMouseDelta -> Steam 0x0072f110  in candidate callees: no
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x00459080 CClientExoAppInternal::ProcessInput -> Steam 0x007ac470  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x188,0x18c}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x10}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0072fd10: 16-byte aligned: YES
  0072fd10  55             PUSH EBP
  0072fd11  8B EC          MOV EBP,ESP
  0072fd13  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0072fd10
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x1  via EAX
  +0x188 [dec 392]  x1  via EDX
  +0x18C [dec 396]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0072fd10 FUN_0072fd10 size=81 ret=4 callers=1 callees=0

undefined4 __thiscall FUN_0072fd10(int param_1,int param_2)

{
  undefined4 local_14;
  
  if ((param_2 < *(int *)(*(int *)(param_1 + 4) + 0x18c)) &&
     (*(int *)(*(int *)(*(int *)(param_1 + 4) + 0x188) + param_2 * 4) != 0)) {
    local_14 = 1;
  }
  else {
    local_14 = 0;
  }
  return local_14;
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
- size 81 B, RET 4
- callers 1: 0x00785df0 CClientExoAppInternal::SetEventDescriptions
- twins: none

## Card B20-1-05

GOG function: CClientAIMaster::AddObject  gog 0x0044eeb0  callconv=(blank)  param_size_bytes=4  gog_size=36  gog_retn=4
Candidate Steam address: 0x007a2510 (size 36, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 5 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 5 (callers: HandleServerToPlayerPlaceableUpdate_Add 0x80d050, ? 0x829850, Constructor 0x86d770)
- best decoy (neighbour/other candidate) scores: 0x007a2540:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044ed40 CClientAIMaster::UpdateState  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0044eee0 CClientAIMaster::RemoveObject  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044ead0 CClientAIList::AddObject -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (5 of 13): the card's A-CALLEE credit may come from these, not from callees
  0x004a1640 CSWCMessage::HandleServerToPlayerPlaceableUpdate_Add -> Steam 0x0080d050  in candidate callers: yes
  0x004bdea0 CSWCMessage::HandleServerToPlayerSafeProjectileSpell -> Steam 0x00829850  in candidate callers: yes
  0x004f90c0 CSWCVisualEffect::Constructor -> Steam 0x0086d770  in candidate callers: yes
  0x0055b480 CSWCSoundObject::AddToArea -> Steam 0x00800cb0  in candidate callers: yes
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
- callers 13: 0x007fddf0, 0x00800cb0 CSWCSoundObject::AddToArea, 0x00804ad0, 0x00806960, 0x0080c6b0, ...
- callees: 0x007a2110 (CALL)
- twins: 0x007a2540 (distinguishing: 0x007a2523: CALL 0x007a2110)

## Card B20-1-06

GOG function: CSWGuiLegalScreen::Destructor  gog 0x0053a810  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x0087fdf0 (size 44, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0087fe20:STR0/CALLEE0/SIG0.05/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0053a760 CSWGuiLegalScreen::Constructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0053ab70 CSWDemoScreen::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0090cce0 CSWGuiPanel::Destructor -> Steam 0x0040e860  in candidate callees: yes
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
prologue for 0x0087fdf0: 16-byte aligned: YES
  0087fdf0  55             PUSH EBP
  0087fdf1  8B EC          MOV EBP,ESP
  0087fdf3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0087fdf0
string literals (0):
direct callees (2):
  0x0040E860  CSWGuiPanel::Destructor
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0087fdf0 FUN_0087fdf0 size=44 ret=4 callers=0 callees=2

undefined4 __thiscall FUN_0087fdf0(undefined4 param_1,uint param_2)

{
  FUN_0040e860();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x0087fe20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0087fe20: 16-byte aligned: YES
  0087fe20  55             PUSH EBP
  0087fe21  8B EC          MOV EBP,ESP
  0087fe23  83 EC 50       SUB ESP,0x50
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

### Static facts (script output, report-only)
- size 44 B, RET 4
- callees: 0x0040e860 CSWGuiPanel::Destructor (CALL), 0x009196fd operator delete (CALL)
- twins: none

## Card B20-1-07

GOG function: CSWCCreature::`scalar  gog 0x00410e50  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x00765180 (size 44, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callees: ? 0x7651b0)
- best decoy (neighbour/other candidate) scores: 0x007651b0:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00410e20 CSWCCreature::IsHostile -> 0x00765150  trusted
- GOG-order neighbour after:  0x00410e80 CSWCCreature::Destructor -> 0x007651b0  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00410e80 CSWCCreature::Destructor -> Steam 0x007651b0  in candidate callees: yes
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
prologue for 0x00765180: 16-byte aligned: YES
  00765180  55             PUSH EBP
  00765181  8B EC          MOV EBP,ESP
  00765183  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00765180
string literals (0):
direct callees (2):
  0x007651B0  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00765180 FUN_00765180 size=44 ret=4 callers=0 callees=2

undefined4 __thiscall FUN_00765180(undefined4 param_1,uint param_2)

{
  FUN_007651b0();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x007651b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007651b0: 16-byte aligned: YES
  007651b0  55             PUSH EBP
  007651b1  8B EC          MOV EBP,ESP
  007651b3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007651b0
string literals (0):
direct callees (9):
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x009196FD  operator delete
  0x00859500  (unnamed)
  0x00623320  CSWSEncounter::AIUpdate
  0x0076D190  CSWCCreature::UnloadModel
  0x007645D0  (unnamed)
  0x00773970  CSWCCreature::SetSpecialWalkAnimation_2
  0x00740A50  CClientExoApp::ClearCachedCreatures
  0x007ED5F0  CSWCObject::Destructor_2
struct offsets touched (top 10):
  +0x178 [dec 376]  x4  via EAX,ECX
  +0x164 [dec 356]  x3  via EAX,ECX,EDX
  +0x184 [dec 388]  x3  via EAX,ECX,EDX
  +0x310 [dec 784]  x3  via EAX,ECX,EDX
  +0x324 [dec 804]  x3  via ECX,EDX
  +0x384 [dec 900]  x3  via EAX,ECX,EDX
  +0x4C [dec 76]  x2  via EAX,EDX
  +0x12C [dec 300]  x2  via EAX,EDX
  +0x130 [dec 304]  x2  via ECX,EDX
  +0x134 [dec 308]  x2  via EAX,ECX
```

### Static facts (script output, report-only)
- size 44 B, RET 4
- callees: 0x007651b0 (CALL), 0x009196fd operator delete (CALL)
- twins: none

## Card B20-1-08

GOG function: CExoArrayList_Vector_::Allocate  gog 0x00457b00  callconv=(blank)  param_size_bytes=4  gog_size=90  gog_retn=4
Candidate Steam address: 0x004292d0 (size 90, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x004291c0:STR0/CALLEE0/SIG0.625/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00457ac0 CExoArrayList_Vector_::CExoArrayList  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00457b60 CExoArrayList_CSWCMapPin__::Allocate  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00457bc0 unnamed -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 11): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
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
prologue for 0x004292d0: 16-byte aligned: YES
  004292d0  55             PUSH EBP
  004292d1  8B EC          MOV EBP,ESP
  004292d3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004292d0
string literals (0):
direct callees (2):
  0x0091C6AA  operator delete[]
  0x004291C0  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 004292d0 FUN_004292d0 size=90 ret=4 callers=11 callees=2

void __thiscall FUN_004292d0(int *param_1,int param_2)

{
  if (param_2 == 0) {
    if (*param_1 != 0) {
      FUN_0091c6aa(*param_1);
      *param_1 = 0;
    }
    param_1[2 = byte +0x8] = 0;
  }
  else {
    FUN_004291c0(param_2);
  }
  param_1[1 = byte +0x4] = param_2;
  return;
}
```

### decoy: 0x004291c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004291c0: 16-byte aligned: YES
  004291c0  55             PUSH EBP
  004291c1  8B EC          MOV EBP,ESP
  004291c3  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004291c0
string literals (0):
direct callees (2):
  0x0091D288  operator new[]
  0x0091C6AA  operator delete[]
struct offsets touched (top 2):
  +0x8 [dec 8]  x5  via ECX,EDX
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
```

### Static facts (script output, report-only)
- size 90 B, RET 4
- callers 11: 0x00429010, 0x0044f5b0, 0x00451460, 0x004545d0, 0x00454df0, ...
- callees: 0x0091c6aa operator delete[] (CALL), 0x004291c0 (CALL)
- twins: none

## Card B20-1-09

GOG function: CExoFile::ReadAsyncComplete  gog 0x0060baa0  callconv=(blank)  param_size_bytes=0  gog_size=21  gog_retn=0
Candidate Steam address: 0x00739b70 (size 69, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00739b20:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0060ba70 CExoFile::ReadAsync  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0060bac0 CExoFile::Write  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00610300 CExoFileInternal::ReadAsyncComplete -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x0061b580 CExoResMan::Demand -> Steam 0x00711c20  in candidate callers: no
  0x0061dc60 CExoResMan::Update -> Steam 0x00714300  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
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
prologue for 0x00739b70: 16-byte aligned: YES
  00739b70  55             PUSH EBP
  00739b71  8B EC          MOV EBP,ESP
  00739b73  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00739b70
string literals (0):
direct callees (1):
  0x0091A825  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00739b70 FUN_00739b70 size=69 ret=0 callers=1 callees=1

bool __fastcall FUN_00739b70(undefined4 *param_1)

{
  int iVar1;
  bool bVar2;
  
  if (*(int *)*param_1 == 0) {
    bVar2 = false;
  }
  else {
    iVar1 = _fseek [GHIDRA-AUTO-LABEL: unreliable](*(FILE **)*param_1,0,0);
    bVar2 = iVar1 == 0;
  }
  return bVar2;
}
```

### decoy: 0x00739b20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00739b20: 16-byte aligned: YES
  00739b20  55             PUSH EBP
  00739b21  8B EC          MOV EBP,ESP
  00739b23  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00739b20
string literals (0):
direct callees (1):
  0x0091A825  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 69 B, RET 0
- callers 1: 0x0071d5e0
- callees: 0x0091a825 (CALL)
- twins: none

## Card B20-1-10

GOG function: CExoFile::Read  gog 0x0060ba20  callconv=(blank)  param_size_bytes=12  gog_size=35  gog_retn=12
Candidate Steam address: 0x007397f0 (size 140, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 6 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00739880:STR0/CALLEE6/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0060ba00 CExoFile::GetSize  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0060ba50 CExoFile::Read_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00610220 CExoFileInternal::Read_2 -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (7 of 22): the card's A-CALLEE credit may come from these, not from callees
  0x0045cea0 CClientExoAppInternal::ReadPlayBackInputEvents -> Steam 0x007b0290  in candidate callers: no
  0x0061cd00 CExoResMan::ServiceFromDirectory -> Steam 0x007133a0  in candidate callers: yes
  0x0061df90 CExoResMan::ServiceFromDirectoryRaw -> Steam 0x00714630  in candidate callers: yes
  0x0061f0a0 CExoResMan::FileCopy -> Steam 0x00715740  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0xc*} | Steam {0x4,0xc*,0x10}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x007397f0: 16-byte aligned: YES
  007397f0  55             PUSH EBP
  007397f1  8B EC          MOV EBP,ESP
  007397f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007397f0
string literals (0):
direct callees (2):
  0x00919723  operator new
  0x0073DA40  Global::operator/
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007397f0 FUN_007397f0 size=140 ret=12 callers=34 callees=2

undefined4 * __thiscall
FUN_007397f0(undefined4 *param_1,undefined4 param_2,undefined2 param_3,undefined4 param_4)

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
  iVar1 = FUN_00919723(0xc,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (iVar1 == 0) {
    local_20 = 0;
  }
  else {
    local_20 = FUN_0073da40(param_2,param_3,param_4);
  }
  *param_1 = local_20;
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x00739880 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00739880: 16-byte aligned: YES
  00739880  55             PUSH EBP
  00739881  8B EC          MOV EBP,ESP
  00739883  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00739880
string literals (0):
direct callees (2):
  0x0073DB30  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 140 B, RET 12
- callers 34: 0x005308b0 CServerExoAppInternal::LoadCharacterStart, 0x00531c50 CServerExoAppInternal::LoadModule, 0x005334e0 CServerExoAppInternal::SetEstimatedSaveSize, 0x00632220, 0x00638bd0, ...
- callees: 0x00919723 operator new (CALL), 0x0073da40 (CALL)
- twins: none

## Card B20-1-11

GOG function: Global::AurMessageParseFloat  gog 0x008c5350  callconv=(blank)  param_size_bytes=8  gog_size=32  gog_retn=0
Candidate Steam address: 0x00484e20 (size 159, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00484e00:STR0/CALLEE3/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x008c5310 Global::AurMessageParseString  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008c5370 Global::AurMessageParseIntOrBool  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x009243dd Global::sscanf -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (3 of 18): the card's A-CALLEE credit may come from these, not from callees
  0x008161b0 CAurTextureBasic::ParseField -> Steam 0x00423ab0  in candidate callers: yes
  0x008c7500 MdlNodeLight::InternalParseField -> Steam 0x00486fb0  in candidate callers: yes
  0x008c7e00 MdlNodeEmitter::InternalParseField -> Steam 0x004878b0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0xc*} | Steam {0x8,0xc*,0x48}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 0)
```

### prologue
```
prologue for 0x00484e20: 16-byte aligned: YES
  00484e20  55             PUSH EBP
  00484e21  8B EC          MOV EBP,ESP
  00484e23  83 EC 48       SUB ESP,0x48
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00484e20
string literals (0):
direct callees (3):
  0x0091C7AD  (unnamed)
  0x009370C7  _stricmp
  0x009196EE  __security_check_cookie
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00484e20 FUN_00484e20 size=159 ret=0 callers=14 callees=3

void FUN_00484e20(char *param_1,undefined4 *param_2)

{
  int iVar1;
  char local_4c [64];
  uint local_c;
  int local_8;
  
  local_c = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  local_8 = FID_conflict__sscanf [GHIDRA-AUTO-LABEL: unreliable](param_1,"%d",param_2);
  if (local_8 == 0) {
    local_4c[0] = '\0';
    FID_conflict__sscanf [GHIDRA-AUTO-LABEL: unreliable](param_1,"%s",local_4c);
    iVar1 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("TRUE",local_4c);
    if (iVar1 == 0) {
      *param_2 = 1;
      local_8 = 1;
    }
    iVar1 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("FALSE",local_4c);
    if (iVar1 == 0) {
      *param_2 = 0;
      local_8 = 1;
    }
  }
  __security_check_cookie [GHIDRA-AUTO-LABEL: unreliable](local_c ^ (uint)&stack0xfffffffc);
  return;
}
```

### decoy: 0x00484e00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00484e00: 16-byte aligned: YES
  00484e00  55             PUSH EBP
  00484e01  8B EC          MOV EBP,ESP
  00484e03  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00484e00
string literals (0):
direct callees (1):
  0x0091C7AD  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 159 B, RET 0
- callers 14: 0x00423ab0 CAurTextureBasic::ParseField, 0x00424f60, 0x00470000, 0x00484ec0, 0x00486fb0 MdlNodeLight::InternalParseField, ...
- callees: 0x0091c7ad (CALL), 0x009370c7 _stricmp (CALL), 0x009196ee __security_check_cookie (CALL)
- twins: none

## Card B20-1-12

GOG function: CResRef::operator==_3  gog 0x0061a570  callconv=(blank)  param_size_bytes=4  gog_size=113  gog_retn=4
Candidate Steam address: 0x00711170 (size 44, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CRes::Destructor | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007110d0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061a4c0 CResRef::operator==_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0061a5f0 CResRef::operator=  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00928a62 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (10 of 40): the card's A-CALLEE credit may come from these, not from callees
  0x00411610 CSWCCreature::SetSoundSet -> Steam 0x00765940  in candidate callers: no
  0x00424b60 CSWCCreature::GetEquipmentForcePowers_2 -> Steam 0x00778ba0  in candidate callers: no
  0x00475550 CSWCObject::SetEnvMap -> Steam 0x007f03f0  in candidate callers: no
  0x004756d0 CSWCObject::SetEnvMap_2 -> Steam 0x007f0570  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*,0x10} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00711170: 16-byte aligned: YES
  00711170  55             PUSH EBP
  00711171  8B EC          MOV EBP,ESP
  00711173  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00711170
string literals (0):
direct callees (2):
  0x007111A0  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00711170 FUN_00711170 size=44 ret=4 callers=0 callees=2

undefined4 __thiscall FUN_00711170(undefined4 param_1,uint param_2)

{
  FUN_007111a0();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x007110d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007110d0: 16-byte aligned: YES
  007110d0  55             PUSH EBP
  007110d1  8B EC          MOV EBP,ESP
  007110d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007110d0
string literals (0):
direct callees (0):
struct offsets touched (top 10):
  +0xC [dec 12]  x5  via EAX,ECX,EDX
  +0x4 [dec 4]  x1  via EDX
  +0x6 [dec 6]  x1  via EDX
  +0x8 [dec 8]  x1  via EAX
  +0x10 [dec 16]  x1  via ECX
  +0x14 [dec 20]  x1  via EAX
  +0x18 [dec 24]  x1  via EAX
  +0x1C [dec 28]  x1  via ECX
  +0x20 [dec 32]  x1  via EDX
  +0x24 [dec 36]  x1  via EAX
```

### Static facts (script output, report-only)
- size 44 B, RET 4
- callees: 0x007111a0 (CALL), 0x009196fd operator delete (CALL)
- twins: none

## Card B20-1-13

GOG function: Global::AurMessageParseFloat  gog 0x008c5350  callconv=(blank)  param_size_bytes=8  gog_size=32  gog_retn=0
Candidate Steam address: 0x00484dc0 (size 52, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00484e00:STR0/CALLEE3/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x008c5310 Global::AurMessageParseString  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008c5370 Global::AurMessageParseIntOrBool  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x009243dd Global::sscanf -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (3 of 18): the card's A-CALLEE credit may come from these, not from callees
  0x008161b0 CAurTextureBasic::ParseField -> Steam 0x00423ab0  in candidate callers: yes
  0x008c7500 MdlNodeLight::InternalParseField -> Steam 0x00486fb0  in candidate callers: no
  0x008c7e00 MdlNodeEmitter::InternalParseField -> Steam 0x004878b0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0xc*} | Steam {0x8,0xc*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 0)
```

### prologue
```
prologue for 0x00484dc0: 16-byte aligned: YES
  00484dc0  55             PUSH EBP
  00484dc1  8B EC          MOV EBP,ESP
  00484dc3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00484dc0
string literals (0):
direct callees (2):
  0x0091C7AD  (unnamed)
  0x0091CFF0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00484dc0 FUN_00484dc0 size=52 ret=0 callers=9 callees=2

void __thiscall FUN_00484dc0(undefined4 param_1,char *param_2,undefined4 param_3)

{
  int iVar1;
  
  iVar1 = FID_conflict__sscanf [GHIDRA-AUTO-LABEL: unreliable](param_2,"%s",param_3,param_1);
  if (iVar1 == 0) {
    FUN_0091cff0(param_3,0);
  }
  return;
}
```

### decoy: 0x00484e00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00484e00: 16-byte aligned: YES
  00484e00  55             PUSH EBP
  00484e01  8B EC          MOV EBP,ESP
  00484e03  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00484e00
string literals (0):
direct callees (1):
  0x0091C7AD  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 52 B, RET 0
- callers 9: 0x00423ab0 CAurTextureBasic::ParseField, 0x0045bf50 Gob::EnableRenderBumpedOut, 0x00485a40, 0x004878b0 MdlNodeEmitter::InternalParseField, 0x0048b6f0, ...
- callees: 0x0091c7ad (CALL), 0x0091cff0 (CALL)
- twins: none

## Card B20-1-14

GOG function: CSWGuiOptionsMouse::HandleInputEvent  gog 0x005725e0  callconv=(blank)  param_size_bytes=8  gog_size=413  gog_retn=8
Candidate Steam address: 0x007dccd0 (size 413, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.73) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007dcb00:STR0/CALLEE0/SIG0.079/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00572410 CSWGuiOptionsMouse::Destructor_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005727b0 CSWGuiOptionsMouse::OnRevMouseButtons  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00572940 CSWGuiOptionsMouse::OnDefault -> Steam ?  in candidate callees: n/a
  0x0090cf20 CSWGuiPanel::HandleInputEvent -> Steam 0x0040eaa0  in candidate callees: yes
  0x0090ee40 CSWGuiManager::PopModalPanel -> Steam 0x004109c0  in candidate callees: yes
  0x00910720 CSWGuiManager::PlayGuiSound -> Steam 0x004122a0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 8 common): GOG {0x1c*,0x20*,0x3c*,0x48*,0x5c*,0x1d4*,0x2f8*,0x808*} | Steam {0x1c*,0x20*,0x3c*,0x48*,0x5c*,0x1d4*,0x2f8*,0x808*}
imm sets (* = in both, 14 common): GOG {0x3*,0x7*,0x8*,0x12*,0x14*,0x28*,0x31*,0x32*,0x200*,0x2f8*,0x7b8*,0xfffffcff*,0xffffff7f*,0xfffffffe*} | Steam {0x3*,0x7*,0x8*,0x12*,0x14*,0x28*,0x31*,0x32*,0x200*,0x2f8*,0x7b8*,0xfffffcff*,0xffffff7f*,0xfffffffe*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x007dccd0: 16-byte aligned: YES
  007dccd0  55             PUSH EBP
  007dccd1  8B EC          MOV EBP,ESP
  007dccd3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007dccd0
string literals (0):
direct callees (4):
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x007DD050  (unnamed)
  0x004109C0  CSWGuiManager::PopModalPanel
  0x0040EAA0  CSWGuiPanel::HandleInputEvent
struct offsets touched (top 9):
  +0x20 [dec 32]  x7  via EAX,ECX,EDX
  +0x1C [dec 28]  x4  via ECX,EDX
  +0x48 [dec 72]  x4  via EAX,ECX
  +0x5C [dec 92]  x3  via EAX,EDX
  +0x1D4 [dec 468]  x3  via EAX,ECX
  +0x3C [dec 60]  x2  via EAX,EDX
  +0x2F8 [dec 760]  x2  via EAX,EDX
  +0x808 [dec 2056]  x1  via EDX
  +0x7DCE88 [dec 8244872]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007dccd0 FUN_007dccd0 size=413 ret=8 callers=0 callees=4

void __thiscall FUN_007dccd0(int param_1,undefined4 param_2,int param_3)

{
  uint uVar1;
  int iVar2;
  
  if (param_3 != 0) {
    switch(param_2) {
    case 0x28:
    case 0x2e:
      FUN_004122a0(0);
      FUN_004109c0();
      *(uint *)(param_1 + 0x48) = *(uint *)(param_1 + 0x48) & 0xffffff7f;
      *(uint *)(param_1 + 0x48) = *(uint *)(param_1 + 0x48) & 0xfffffcff | 0x200;
      break;
    case 0x2a:
      FUN_004122a0(0);
      *(undefined4 *)(param_1 + 0x808) = 1;
      FUN_007dd050(param_1 + 0x7b8);
      break;
    case 0x2d:
      if ((*(int *)(param_1 + 0x20) != 0) &&
         (iVar2 = (**(code **)(**(int **)(param_1 + 0x20) + 0x5c))(), iVar2 != 0)) {
        FUN_004122a0(3);
        iVar2 = (**(code **)(**(int **)(param_1 + 0x20) + 0x5c))();
        uVar1 = *(uint *)(iVar2 + 0x1d4);
        iVar2 = (**(code **)(**(int **)(param_1 + 0x20) + 0x5c))();
        *(uint *)(iVar2 + 0x1d4) = *(uint *)(iVar2 + 0x1d4) & 0xfffffffe | (uint)((uVar1 & 1) == 0);
      }
      break;
    case 0x39:
      (**(code **)(*(int *)(param_1 + 0x2f8) + 0x3c))(0x31,1);
      break;
    case 0x3a:
      (**(code **)(*(int *)(param_1 + 0x2f8) + 0x3c))(0x32,1);
    }
  }
  FUN_0040eaa0(param_2,param_3);
  return;
}
```

### decoy: 0x007dcb00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007dcb00: 16-byte aligned: YES
  007dcb00  55             PUSH EBP
  007dcb01  8B EC          MOV EBP,ESP
  007dcb03  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007dcb00
string literals (0):
direct callees (8):
  0x00919BCC  `eh vector destructor iterator` (CRT)
  0x00414C60  CSWGuiBorder::Destructor_2
  0x0041A3D0  CSWGuiButton::Destructor
  0x00417290  (unnamed)
  0x00418640  CSWGuiControl::Destructor
  0x0041AC10  (unnamed)
  0x0041D190  (unnamed)
  0x0040E860  CSWGuiPanel::Destructor
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 413 B, RET 8
- callees: 0x004122a0 CSWGuiManager::PlayGuiSound (CALL), 0x007dd050 (CALL), 0x004109c0 CSWGuiManager::PopModalPanel (CALL), 0x0040eaa0 CSWGuiPanel::HandleInputEvent (CALL)
- indirect: 0x007dcdac slot 0x5c, 0x007dcdd0 slot 0x5c, 0x007dcdf5 slot 0x5c, 0x007dce38 slot 0x3c, 0x007dce55 slot 0x3c
- twins: none

## Card B20-1-15

GOG function: CSWGuiInGameMessages::ShowEffects  gog 0x0052bec0  callconv=(blank)  param_size_bytes=0  gog_size=1659  gog_retn=None
Candidate Steam address: 0x0075a7c0 (size 1660, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: n/a | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 3 (callees: Constructor 0x414a50, SetDefaultColor 0x417140, SetSelectedControl 0x41e870); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0075a630:STR0/CALLEE1/SIG0.427/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0052bd30 CSWGuiInGameMessages::ShowCombatMessages  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0052c53b Global::SWMSGSetupEffectRow  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (24), mapped to Steam through trusted rows only:
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam 0x0073f750  in candidate callees: yes
  0x0040d070 CClientExoApp::GetSWParty -> Steam 0x0073fb90  in candidate callees: yes
  0x00429780 CSWCCreature::GetServerCreature -> Steam 0x0077d800  in candidate callees: yes
  0x004789d0 CSWParty::GetCharacter -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x005340d0 CExoArrayList_CSWGuiControl__::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x00534170 CExoArrayList::Add_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  ... 16 more GOG callees
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 12 common): GOG {0x4*,0x8*,0x14*,0x20*,0x40*,0x48*,0xa0*,0x2a4*,0x2d0*,0xc14*,0xf04*,0xfc8*} | Steam {0x4*,0x8*,0x14*,0x20*,0x40*,0x48*,0xa0*,0x2a4*,0x2d0*,0xc14*,0xf04*,0xfc8*}
imm sets (* = in both, 24 common): GOG {0x2*,0x3*,0x4*,0x5*,0xa*,0x68*,0xcc*,0xd8*,0xf0*,0x148*,0x15c*,0x1c0*,0x358*,0x648*,...(+10)} | Steam {0x2*,0x3*,0x4*,0x5*,0xa*,0x68*,0xcc*,0xd8*,0xf0*,0x148*,0x15c*,0x1c0*,0x358*,0x648*,...(+10)}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0075a7c0: 16-byte aligned: YES
  0075a7c0  55             PUSH EBP
  0075a7c1  8B EC          MOV EBP,ESP
  0075a7c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 0075a7c0 FUN_0075a7c0 size=1660 ret=0 callers=3 callees=24

void __fastcall FUN_0075a7c0(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  int *local_d4;
  undefined1 local_78 [8];
  int local_70;
  int local_6c;
  int local_68;
  undefined1 local_64 [8];
  undefined1 local_5c [4];
  int local_58;
  undefined4 local_50;
  undefined4 local_4c;
  int local_48;
  int local_44;
  undefined4 local_40;
  undefined1 local_3c [4];
  int local_38;
  int *local_30;
  int *local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00963ea3;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *(uint *)(param_1 + 0xb0) = *(uint *)(param_1 + 0xb0) & 0xfffffffd;
  *(uint *)(param_1 + 0x3a0) = *(uint *)(param_1 + 0x3a0) & 0xfffffffd;
  *(uint *)(param_1 + 0x980) = *(uint *)(param_1 + 0x980) & 0xfffffffd | 2;
  *(uint *)(param_1 + 0xc70) = *(uint *)(param_1 + 0xc70) & 0xfffffffd | 2;
  *(uint *)(param_1 + 0x690) = *(uint *)(param_1 + 0x690) & 0xfffffffd;
  *(uint *)(param_1 + 0x10b4) = *(uint *)(param_1 + 0x10b4) & 0xfffffffd | 2;
  *(uint *)(param_1 + 0x11fc) = *(uint *)(param_1 + 0x11fc) & 0xfffffffd | 2;
  FUN_00417140(&DAT_00a7fe24);
  FUN_00417140(&DAT_00a7fe24);
  FUN_00417140(&DAT_00a7fe24);
  FUN_00417140(&DAT_00a7fdac);
  FUN_0041d290();
  FUN_0041d290();
  uVar3 = 0;
  FUN_0073fb90(0);
  local_14 = FUN_007e5da0(uVar3);
  if ((local_14 != 0) && (local_6c = FUN_0077d800(), local_6c != 0)) {
    local_2c = (int *)(local_6c + 0xffc);
    FUN_0084a640(0);
    local_8 = 0;
    FUN_0084a640(0);
    local_8._0_1_ = 1;
    local_68 = 0;
    local_28 = *(undefined4 *)(param_1 + 0xc14);
    local_1c = *(int *)(param_1 + 0xbdc) + (uint)*(byte *)(param_1 + 0xc08) * -2;
    local_24 = 0;
    local_20 = 0;
    local_18 = 10;
    local_40 = *(undefined4 *)(param_1 + 0xf04);
    local_48 = *(int *)(param_1 + 0xecc) + (uint)*(byte *)(param_1 + 0xef8) * -2;
    local_50 = 0;
    local_4c = 0;
    local_44 = 10;
    local_30 = (int *)0x0;
    FUN_00733540();
    local_8._0_1_ = 2;
    for (local_70 = 0; local_70 < local_2c[1 = byte +0x4]; local_70 = local_70 + 1) {
      local_68 = *(int *)(*local_2c + local_70 * 4);
      if (*(int *)(local_68 + 0x20) != 0) {
        uVar3 = FUN_0071dee0(local_78,*(undefined4 *)(local_68 + 0x20));
        local_8._0_1_ = 3;
        FUN_007337c0(uVar3);
/* ... 59 more lines truncated by --max-lines */
```

### decoy: 0x0075a630 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0075a630: 16-byte aligned: YES
  0075a630  55             PUSH EBP
  0075a631  8B EC          MOV EBP,ESP
  0075a633  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

### Static facts (script output, report-only)
- size 1660 B, RET 0
- callers 3: 0x00759d20, 0x0075a1f0, 0x0075a2b0
- callees: 0x00417140 CSWGuiTextParams::SetDefaultColor (CALL), 0x0041d290 (CALL), 0x0073fb90 CClientExoApp::GetSWParty (CALL), 0x007e5da0 Party member entry thiscall(party, int idx) RET 4 (0x7E5D80 = idx 0) (CALL), 0x0077d800 (CALL), 0x0084a640 (CALL), ...
- indirect: 0x0075ac66 slot 0x8, 0x0075ad0c slot 0x8
- globals: 0x00a1b4b0 READ
- twins: none

## Card B20-1-16

GOG function: CExoFile::ReadAsyncComplete  gog 0x0060baa0  callconv=(blank)  param_size_bytes=0  gog_size=21  gog_retn=0
Candidate Steam address: 0x00739970 (size 21, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00739950:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0060ba70 CExoFile::ReadAsync  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0060bac0 CExoFile::Write  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00610300 CExoFileInternal::ReadAsyncComplete -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x0061b580 CExoResMan::Demand -> Steam 0x00711c20  in candidate callers: no
  0x0061dc60 CExoResMan::Update -> Steam 0x00714300  in candidate callers: no
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
prologue for 0x00739970: 16-byte aligned: YES
  00739970  55             PUSH EBP
  00739971  8B EC          MOV EBP,ESP
  00739973  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00739970
string literals (0):
direct callees (1):
  0x0073DBA0  CExoFileInternal::GetSize
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00739970 FUN_00739970 size=21 ret=0 callers=14 callees=1
THUNK -> 0x0073DBA0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_00739970(undefined4 param_1)

{
  FUN_0073dba0(param_1);
  return;
}
```

### decoy: 0x00739950 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00739950: 16-byte aligned: YES
  00739950  55             PUSH EBP
  00739951  8B EC          MOV EBP,ESP
  00739953  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00739950
string literals (0):
direct callees (1):
  0x0073DB70  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 21 B, RET 0
- callers 14: 0x00531c50 CServerExoAppInternal::LoadModule, 0x005334e0 CServerExoAppInternal::SetEstimatedSaveSize, 0x00632220, 0x00638bd0, 0x007133a0 CExoResMan::ServiceFromDirectory, ...
- callees: 0x0073dba0 CExoFileInternal::GetSize (CALL)
- twins: 0x00739950, 0x00739a10, 0x00739cf0, 0x00739d10 (distinguishing: 0x0073997c: CALL 0x0073dba0)

## Card B20-1-17

GOG function: CExoFile::Read  gog 0x0060ba20  callconv=(blank)  param_size_bytes=12  gog_size=35  gog_retn=12
Candidate Steam address: 0x007399e0 (size 35, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: ServiceFromDirectory 0x7133a0)
- best decoy (neighbour/other candidate) scores: 0x007399c0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0060ba00 CExoFile::GetSize  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0060ba50 CExoFile::Read_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00610220 CExoFileInternal::Read_2 -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (7 of 22): the card's A-CALLEE credit may come from these, not from callees
  0x0045cea0 CClientExoAppInternal::ReadPlayBackInputEvents -> Steam 0x007b0290  in candidate callers: no
  0x0061cd00 CExoResMan::ServiceFromDirectory -> Steam 0x007133a0  in candidate callers: yes
  0x0061df90 CExoResMan::ServiceFromDirectoryRaw -> Steam 0x00714630  in candidate callers: no
  0x0061f0a0 CExoResMan::FileCopy -> Steam 0x00715740  in candidate callers: no
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
prologue for 0x007399e0: 16-byte aligned: YES
  007399e0  55             PUSH EBP
  007399e1  8B EC          MOV EBP,ESP
  007399e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007399e0
string literals (0):
direct callees (1):
  0x0073DDD0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007399e0 FUN_007399e0 size=35 ret=12 callers=3 callees=1
THUNK -> 0x0073DDD0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_007399e0(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  FUN_0073ddd0(param_1,param_2,param_3);
  return;
}
```

### decoy: 0x007399c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007399c0: 16-byte aligned: YES
  007399c0  55             PUSH EBP
  007399c1  8B EC          MOV EBP,ESP
  007399c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007399c0
string literals (0):
direct callees (1):
  0x0073DC30  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 35 B, RET 12
- callers 3: 0x007133a0 CExoResMan::ServiceFromDirectory, 0x007279f0, 0x00729420
- callees: 0x0073ddd0 (CALL)
- twins: 0x00739990 (distinguishing: 0x007399f8: CALL 0x0073ddd0)

## Card B20-1-18

GOG function: CExoInput::UpdateMouseDelta  gog 0x00616990  callconv=(blank)  param_size_bytes=4  gog_size=31  gog_retn=4
Candidate Steam address: 0x0072fe30 (size 36, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0072fe10:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00616970 CExoInput::GetMouseDelta  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x006169b0 CExoInput::GetJoystickNStick  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00615ec0 CExoInputInternal::UpdateMouseDelta -> Steam 0x0072f110  in candidate callees: no
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x00459080 CClientExoAppInternal::ProcessInput -> Steam 0x007ac470  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x410}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x8}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0072fe30: 16-byte aligned: YES
  0072fe30  55             PUSH EBP
  0072fe31  8B EC          MOV EBP,ESP
  0072fe33  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0072fe30
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x410 [dec 1040]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0072fe30 FUN_0072fe30 size=36 ret=4 callers=5 callees=0

void __thiscall FUN_0072fe30(int param_1,undefined4 param_2)

{
  *(undefined4 *)(*(int *)(param_1 + 4) + 0x410) = param_2;
  return;
}
```

### decoy: 0x0072fe10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0072fe10: 16-byte aligned: YES
  0072fe10  55             PUSH EBP
  0072fe11  8B EC          MOV EBP,ESP
  0072fe13  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0072fe10
string literals (0):
direct callees (1):
  0x0072F0F0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### Static facts (script output, report-only)
- size 36 B, RET 4
- callers 5: 0x008fff20 CSWGuiInGameOptKeyMappings::Constructor, 0x009007a0 CSWGuiInGameOptKeyMappings::HandleInputEvent, 0x00901310, 0x009014b0, 0x00901590 CSWGuiInGameOptKeyMappings::Update
- twins: none
