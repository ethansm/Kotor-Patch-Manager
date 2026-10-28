# Review cards B18 chunk 1

Answer each card with one TSV row in `steam_db_batches/B18/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B18-1-01

GOG function: CExoSound::StopAllOneShots  gog 0x0063aaa0  callconv=(blank)  param_size_bytes=0  gog_size=29  gog_retn=0
Candidate Steam address: 0x0070b8f0 (size 29, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CExoSound::Render | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0070b8c0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0063aa30 CExoSound::Play3DOneShotSound  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0063aac0 CExoSound::EnableStreamingEnvironmentEffects  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00638c70 CExoSoundInternal::StopAllOneShots -> Steam 0x00709cf0  in candidate callees: no
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x007b3120 CServerExoAppInternal::StartNewModule -> Steam 0x0052fda0  in candidate callers: no
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
prologue for 0x0070b8f0: 16-byte aligned: YES
  0070b8f0  55             PUSH EBP
  0070b8f1  8B EC          MOV EBP,ESP
  0070b8f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0070b8f0
string literals (0):
direct callees (1):
  0x00705D70  CExoSoundInternal::Render (per-frame)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0070b8f0 FUN_0070b8f0 size=29 ret=0 callers=6 callees=1

void __fastcall FUN_0070b8f0(int *param_1)

{
  if (*param_1 != 0) {
    FUN_00705d70();
  }
  return;
}
```

### decoy: 0x0070b8c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0070b8c0: 16-byte aligned: YES
  0070b8c0  55             PUSH EBP
  0070b8c1  8B EC          MOV EBP,ESP
  0070b8c3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0070b8c0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x2D [dec 45]  x1  via EDX
```

### Static facts (script output, report-only)
- size 29 B, RET 0
- callers 6: 0x00409ed0 Global::UpdateScreen, 0x00783ee0 CClientExoAppInternal::UpdateSoundEngine, 0x0078e870 CClientExoAppInternal::FadeAndStopStreamingSoundSource, 0x007a0580 CClientExoAppInternal::BusyWaitMenuMusic, 0x007a0630 CClientExoAppInternal::BusyWaitMuteSound, ...
- callees: 0x00705d70 (CALL)
- twins: 0x0070bb00, 0x0070bbe0 (distinguishing: 0x0070b904: CALL 0x00705d70)

## Card B18-1-02

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

## Card B18-1-03

GOG function: Global::AurGUICloseViewport  gog 0x008d02d0  callconv=(blank)  param_size_bytes=0  gog_size=235  gog_retn=0
Candidate Steam address: 0x00479ab0 (size 92, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=Global::GetResolutionTextScalar | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.07 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004799c0:STR0/CALLEE6/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008cfd70 Global::AurGUISetupViewport  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008d03c0 Global::GetResolutionTextScalar -> 0x00479ab0  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (6 of 12): the card's A-CALLEE credit may come from these, not from callees
  0x0051cb70 CSWGuiMainInterface::Draw -> Steam 0x0074b3f0  in candidate callers: no
  0x00521f10 CSWGuiMainInterface::DrawMap -> Steam 0x00750790  in candidate callers: no
  0x00564c30 CSWGuiMapHider::Draw -> Steam 0x00895cc0  in candidate callers: yes
  0x0090d110 CSWGuiPanel::Draw -> Steam 0x0040ec90  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x2,0xa*,0xb,0x10} | Steam {0xa*,0xc,0x3fe,0x3ff}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00479ab0: 16-byte aligned: YES
  00479ab0  55             PUSH EBP
  00479ab1  8B EC          MOV EBP,ESP
  00479ab3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00479ab0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00479ab0 FUN_00479ab0 size=92 ret=0 callers=43 callees=0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 FUN_00479ab0(void)

{
  undefined4 local_10;
  
  if (DAT_009f42a4 + -0x3fe < 0) {
    local_10 = 0;
  }
  else {
    local_10 = DAT_009f42a4 + -0x3fe;
  }
  _DAT_009f5f64 = (float)((int)(local_10 + (local_10 >> 0x1f & 0x3ffU)) >> 10) * _DAT_0098c014 + 1.0
  ;
  return (float10)_DAT_009f5f64;
}
```

### decoy: 0x004799c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004799c0: 16-byte aligned: YES
  004799c0  55             PUSH EBP
  004799c1  8B EC          MOV EBP,ESP
  004799c3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004799c0
string literals (0):
direct callees (0):
struct offsets touched (top 6):
  +0x9F4928 [dec 10438952]  x2  via EAX,ECX
  +0xA309CE [dec 10684878]  x2  via ECX,EDX
  +0xA309C8 [dec 10684872]  x1  via EDX
  +0xA309CA [dec 10684874]  x1  via ECX
  +0xA309CC [dec 10684876]  x1  via EAX
  +0xA309D0 [dec 10684880]  x1  via EDX
```

### Static facts (script output, report-only)
- size 92 B, RET 0
- callers 43: 0x00417910, 0x00479ca0, 0x0047a510, 0x0047b500, 0x007457e0, ...
- twins: none

## Card B18-1-04

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

## Card B18-1-05

GOG function: Global::AurGUICloseViewport  gog 0x008d02d0  callconv=(blank)  param_size_bytes=0  gog_size=235  gog_retn=0
Candidate Steam address: 0x00479460 (size 845, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=GUI | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 6 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004799c0:STR0/CALLEE6/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008cfd70 Global::AurGUISetupViewport  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008d03c0 Global::GetResolutionTextScalar -> 0x00479ab0  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (6 of 12): the card's A-CALLEE credit may come from these, not from callees
  0x0051cb70 CSWGuiMainInterface::Draw -> Steam 0x0074b3f0  in candidate callers: yes
  0x00521f10 CSWGuiMainInterface::DrawMap -> Steam 0x00750790  in candidate callers: yes
  0x00564c30 CSWGuiMapHider::Draw -> Steam 0x00895cc0  in candidate callers: yes
  0x0090d110 CSWGuiPanel::Draw -> Steam 0x0040ec90  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8}
imm sets (* = in both, 4 common): GOG {0x2*,0xa*,0xb*,0x10*} | Steam {0x2*,0x3,0x4,0x5,0x6,0x8,0xa*,0xb*,0x10*,0x18,0x70,0x96000000}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00479460: 16-byte aligned: YES
  00479460  55             PUSH EBP
  00479461  8B EC          MOV EBP,ESP
  00479463  83 EC 70       SUB ESP,0x70
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00479460
string literals (0):
direct callees (2):
  0x00517980  Vec3::Equal
  0x00408740  Global::glClear_glClear_int_engineMask
struct offsets touched (top 9):
  +0x9F4928 [dec 10438952]  x5  via EAX,ECX
  +0xA309C8 [dec 10684872]  x2  via EAX,ECX
  +0xA309CA [dec 10684874]  x2  via EAX,EDX
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via EAX
  +0x9885F4 [dec 9995764]  x1  via EDX
  +0xA309CC [dec 10684876]  x1  via ECX
  +0xA309CE [dec 10684878]  x1  via EAX
  +0xA309D0 [dec 10684880]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00479460 FUN_00479460 size=845 ret=0 callers=12 callees=8

/* WARNING: Switch with 1 destination removed at 0x00479701 */
/* WARNING: Switch with 1 destination removed at 0x0047978a */
/* WARNING: Switch with 1 destination removed at 0x00479813 */
/* WARNING: Switch with 1 destination removed at 0x0047989c */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4
FUN_00479460(int param_1,int param_2,int param_3,int param_4,float *param_5,byte param_6,
            float param_7)

{
  undefined4 uVar1;
  int iVar2;
  uint local_60;
  float local_5c;
  int *local_58;
  float local_54;
  undefined1 *local_50;
  float local_4c;
  undefined1 *local_48;
  float local_44;
  undefined1 *local_40;
  int local_3c;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined1 local_20 [4];
  uint local_1c;
  undefined1 local_18 [4];
  short local_14;
  short local_10;
  undefined1 local_c [4];
  int local_8;
  
  if (DAT_00a32a30 == 0) {
    if (((((param_1 + param_3 < 0) || (param_2 + param_4 < 0)) || (param_3 < 1)) ||
        ((param_4 < 1 || (DAT_009f42a4 <= param_1)))) || (DAT_009f42a8 <= param_2)) {
      uVar1 = 0;
    }
    else {
      local_14 = *(short *)(&DAT_00a309c8 + DAT_00a309c0 * 10) + (short)param_1;
      local_10 = *(short *)(&DAT_00a309ca + DAT_00a309c0 * 10) + (short)param_2;
      DAT_00a309c0 = DAT_00a309c0 + 1;
      *(short *)(&DAT_00a309c8 + DAT_00a309c0 * 10) = local_14;
      *(short *)(&DAT_00a309ca + DAT_00a309c0 * 10) = local_10;
      *(undefined2 *)(&DAT_00a309cc + DAT_00a309c0 * 10) = (undefined2)param_3;
      (&DAT_00a309ce)[DAT_00a309c0 * 5] = (undefined2)param_4;
      *(ushort *)(&DAT_00a309d0 + DAT_00a309c0 * 10) = (ushort)param_6;
      *(float *)(&DAT_00a30980 + DAT_00a309c0 * 4) = param_7;
      local_3c = ((int)DAT_00a309ce - (int)local_10) - param_4;
      glViewport((int)local_14,local_3c,param_3,param_4);
      local_44 = *param_5 * (float)_DAT_009884a0;
      local_40 = local_c;
      local_4c = param_5[1 = byte +0x4] * (float)_DAT_009884a0;
      local_48 = local_18;
      local_54 = param_5[2 = byte +0x8] * (float)_DAT_009884a0;
      local_50 = local_20;
      local_5c = param_7 * (float)_DAT_009884a0;
      local_58 = &local_8;
      local_8 = (int)ROUND(ROUND(local_5c));
      local_1c = (int)ROUND(ROUND(local_4c)) << 8 | (int)ROUND(ROUND(local_44)) |
                 (int)ROUND(ROUND(local_54)) << 0x10 | local_8 << 0x18;
      local_2c = _DAT_00987474;
      local_28 = _DAT_00987474;
      local_24 = _DAT_00987474;
      iVar2 = FUN_00517980(param_5,&local_2c);
      if ((iVar2 == 0) && (0 < local_8)) {
        local_38 = _DAT_0098bd70;
        local_34 = _DAT_0098bd70;
        local_30 = _DAT_0098bd70;
        iVar2 = FUN_00517980(param_5,&local_38);
        if (iVar2 != 0) {
          local_1c = 0x96000000;
        }
        glDisable(DAT_009f4938);
        glDisable(DAT_009f493c);
/* ... 23 more lines truncated by --max-lines */
```

### decoy: 0x004799c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004799c0: 16-byte aligned: YES
  004799c0  55             PUSH EBP
  004799c1  8B EC          MOV EBP,ESP
  004799c3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004799c0
string literals (0):
direct callees (0):
struct offsets touched (top 6):
  +0x9F4928 [dec 10438952]  x2  via EAX,ECX
  +0xA309CE [dec 10684878]  x2  via ECX,EDX
  +0xA309C8 [dec 10684872]  x1  via EDX
  +0xA309CA [dec 10684874]  x1  via ECX
  +0xA309CC [dec 10684876]  x1  via EAX
  +0xA309D0 [dec 10684880]  x1  via EDX
```

### Static facts (script output, report-only)
- size 845 B, RET 0
- callers 12: 0x0040ec90 CSWGuiPanel::Draw, 0x004113a0, 0x004184f0 CSWGuiScene::Draw, 0x0041bf40 CSWGuiProgressBarReveal::Draw, 0x0041f980, ...
- callees: 0x0000004f (CALL), 0x00517980 (CALL), 0x0000001c (CALL), 0x0000002d (CALL), 0x0000002c (CALL), 0x00000029 (CALL), ...
- globals: 0x00a309c0 READ, 0x00a309c0 READ, 0x00a309c0 READ, ...
- imports: glBegin (OPENGL32.DLL), glColor4ubv (OPENGL32.DLL), glDisable (OPENGL32.DLL), ...
- twins: none

## Card B18-1-06

GOG function: CExoSound::StopAllOneShots  gog 0x0063aaa0  callconv=(blank)  param_size_bytes=0  gog_size=29  gog_retn=0
Candidate Steam address: 0x0070bb80 (size 42, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0070bb50:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0063aa30 CExoSound::Play3DOneShotSound  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0063aac0 CExoSound::EnableStreamingEnvironmentEffects  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00638c70 CExoSoundInternal::StopAllOneShots -> Steam 0x00709cf0  in candidate callees: no
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x007b3120 CServerExoAppInternal::StartNewModule -> Steam 0x0052fda0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0xdc}
imm sets (* = in both, 0 common): GOG {} | Steam {0x8}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0070bb80: 16-byte aligned: YES
  0070bb80  55             PUSH EBP
  0070bb81  8B EC          MOV EBP,ESP
  0070bb83  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0070bb80
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xDC [dec 220]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0070bb80 FUN_0070bb80 size=42 ret=0 callers=2 callees=0

undefined4 __fastcall FUN_0070bb80(int *param_1)

{
  undefined4 uVar1;
  
  if (*param_1 == 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = *(undefined4 *)(*param_1 + 0xdc);
  }
  return uVar1;
}
```

### decoy: 0x0070bb50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0070bb50: 16-byte aligned: YES
  0070bb50  55             PUSH EBP
  0070bb51  8B EC          MOV EBP,ESP
  0070bb53  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0070bb50
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EAX
```

### Static facts (script output, report-only)
- size 42 B, RET 0
- callers 2: 0x007971a0 CClientExoAppInternal::InitializeSoundOptions, 0x007db680
- twins: none

## Card B18-1-07

GOG function: GLRender::FrameBufferModifications  gog 0x00827e30  callconv=(blank)  param_size_bytes=4  gog_size=1885  gog_retn=0
Candidate Steam address: 0x00435e80 (size 113, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00435720:STR0/CALLEE0/SIG1/SRV0/VT0
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
disp sets (* = in both, 0 common): GOG {0x28} | Steam {0x4,0x8,0xc}
imm sets (* = in both, 0 common): GOG {0x6,0x14} | Steam {0x2,0x8}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
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

### decoy: 0x00435720 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00435720: 16-byte aligned: YES
  00435720  55             PUSH EBP
  00435721  8B EC          MOV EBP,ESP
  00435723  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00435720
string literals (0):
direct callees (2):
  0x004848B0  (unnamed)
  0x00484970  (unnamed)
struct offsets touched (top 1):
  +0x28 [dec 40]  x1  via EDX
```

### Static facts (script output, report-only)
- size 113 B, RET 0
- callers 1: 0x004cdf40
- indirect: 0x00435e9c slot 0x4, 0x00435ebc slot 0x8, 0x00435ee8 slot 0xc
- twins: none

## Card B18-1-08

GOG function: GLRender::FrameBufferModifications  gog 0x00827e30  callconv=(blank)  param_size_bytes=4  gog_size=1885  gog_retn=0
Candidate Steam address: 0x004356e0 (size 54, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00435720:STR0/CALLEE0/SIG1/SRV0/VT0
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
disp sets (* = in both, 0 common): GOG {0x28} | Steam {}
imm sets (* = in both, 0 common): GOG {0x6,0x14} | Steam {0xc,0x18}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
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

### decoy: 0x00435720 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00435720: 16-byte aligned: YES
  00435720  55             PUSH EBP
  00435721  8B EC          MOV EBP,ESP
  00435723  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00435720
string literals (0):
direct callees (2):
  0x004848B0  (unnamed)
  0x00484970  (unnamed)
struct offsets touched (top 1):
  +0x28 [dec 40]  x1  via EDX
```

### Static facts (script output, report-only)
- size 54 B, RET 0
- callers 1: 0x00476be0
- callees: 0x00000043 (CALL), 0x0000003d (CALL), 0x0091c630 (CALL)
- imports: wglGetCurrentContext (OPENGL32.DLL), wglGetCurrentDC (OPENGL32.DLL)
- twins: none

## Card B18-1-09

GOG function: Global::AurResGetNextLine  gog 0x00867ee0  callconv=(blank)  param_size_bytes=0  gog_size=69  gog_retn=0
Candidate Steam address: 0x00475ea0 (size 192, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CloseModelStream | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00475f70:STR0/CALLEE4/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00867e10 Global::AurResFree  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00867fa0 Global::AurResFree_2 -> 0x00476030  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00867f30 unnamed -> Steam ?  in candidate callees: n/a
  0x00867fa0 Global::AurResFree_2 -> Steam 0x00476030  in candidate callees: no
GOG callers mapped through trusted rows (3 of 20): the card's A-CALLEE credit may come from these, not from callees
  0x00817210 CAurTextureBasic::Init -> Steam 0x00424b10  in candidate callers: yes
  0x0084e530 Gob::EnableRenderBumpedOut -> Steam 0x0045bf50  in candidate callers: yes
  0x00861e10 Scene::LoadVisibility -> Steam 0x0046fbd0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x18}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x8,0x10}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00475ea0: 16-byte aligned: YES
  00475ea0  55             PUSH EBP
  00475ea1  8B EC          MOV EBP,ESP
  00475ea3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00475ea0
string literals (0):
direct callees (5):
  0x0091C5B3  (unnamed)
  0x00476620  (unnamed)
  0x00449170  (unnamed)
  0x00475A70  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x18 [dec 24]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00475ea0 FUN_00475ea0 size=192 ret=0 callers=11 callees=5

void FUN_00475ea0(int *param_1,undefined4 param_2)

{
  int iVar1;
  
  if (param_1 != (int *)0x0) {
    if (*param_1 == 0) {
      (*DAT_00a30830)(param_1[6 = byte +0x18],param_2);
      param_1[6 = byte +0x18] = 0;
    }
    else {
      _fclose [GHIDRA-AUTO-LABEL: unreliable]((FILE *)*param_1);
      *param_1 = 0;
    }
    iVar1 = FUN_00476620(param_1);
    if (iVar1 != 0) {
      FUN_00449170(param_1);
    }
    if (param_1 != (int *)0x0) {
      FUN_00475a70();
      FUN_009196fd(param_1);
    }
  }
  return;
}
```

### decoy: 0x00475f70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00475f70: 16-byte aligned: YES
  00475f70  55             PUSH EBP
  00475f71  8B EC          MOV EBP,ESP
  00475f73  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00475f70
string literals (0):
direct callees (2):
  0x00475FC0  (unnamed)
  0x00476030  Global::AurResFree_2
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 192 B, RET 0
- callers 11: 0x00424b10 CAurTextureBasic::Init, 0x00452120, 0x0045bf50 Gob::EnableRenderBumpedOut, 0x0046fbd0 Scene::LoadVisibility, 0x00470000, ...
- callees: 0x0091c5b3 (CALL), 0x00476620 (CALL), 0x00449170 (CALL), 0x00475a70 (CALL), 0x009196fd operator delete (CALL)
- indirect: 0x00475eec unresolved
- twins: none

## Card B18-1-10

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

## Card B18-1-11

GOG function: Global::InsertModel  gog 0x008ce680  callconv=(blank)  param_size_bytes=4  gog_size=48  gog_retn=0
Candidate Steam address: 0x0048e160 (size 309, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::FindModel | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.10 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0048e130:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x008ce5e0 Global::BinaryInsertModel -> 0x0048e090  trusted
- GOG-order neighbour after:  0x008ce6b0 Global::FindModel  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x008ce5e0 Global::BinaryInsertModel -> Steam 0x0048e090  in candidate callees: no
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x8*,0xc} | Steam {0x4,0x8*,0x10,0x120}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x0048e160: 16-byte aligned: YES
  0048e160  55             PUSH EBP
  0048e161  8B EC          MOV EBP,ESP
  0048e163  81 EC 20 01 00 00 SUB ESP,0x120
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0048e160
string literals (3):
  0x0098D6B8  at 0048e227  'AR_ERROR.LOG'
  0x0098D6A4  at 0048e248  'Names Differ: %s %s'
  0x0098D6A4  at 0048e267  'Names Differ: %s %s'
direct callees (10):
  0x009370C7  _stricmp
  0x0048E060  (unnamed)
  0x00511ED0  (unnamed)
  0x00511F50  (unnamed)
  0x00452830  (unnamed)
  0x0091CAEB  (unnamed)
  0x00919671  (unnamed)
  0x0091C90B  (unnamed)
  0x0091C5B3  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0048e160 FUN_0048e160 size=309 ret=0 callers=8 callees=10

void FUN_0048e160(char *param_1)

{
  int iVar1;
  char local_124 [260];
  uint local_20;
  FILE *local_1c;
  undefined4 local_18;
  int local_14;
  int local_10;
  int local_c;
  undefined4 local_8;
  
  local_20 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  if (((param_1 != (char *)0x0) && (iVar1 = __stricmp [GHIDRA-AUTO-LABEL: unreliable](param_1,"NULL"), iVar1 != 0)) &&
     (local_10 = FUN_0048e060(param_1), local_10 == 0)) {
    local_18 = DAT_00a32e04;
    DAT_00a32e04 = 0;
    local_8 = FID_conflict__Immortalize [GHIDRA-AUTO-LABEL: unreliable]<std::_Generic_error_category [GHIDRA-AUTO-LABEL: unreliable]>();
    local_c = FUN_00511f50(param_1);
    if (local_c != 0) {
      DAT_00a32e04 = FUN_00452830();
    }
    iVar1 = DAT_00a32e04;
    local_14 = DAT_00a32e04;
    DAT_00a32e04 = local_18;
    if (((iVar1 != 0) && (iVar1 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]((char *)(iVar1 + 8),param_1), iVar1 != 0)) &&
       (local_1c = _fopen [GHIDRA-AUTO-LABEL: unreliable]("AR_ERROR.LOG","w"), local_1c != (FILE *)0x0)) {
      _sprintf [GHIDRA-AUTO-LABEL: unreliable](local_124,"Names Differ: %s %s",local_14 + 8,param_1);
      _fprintf [GHIDRA-AUTO-LABEL: unreliable](local_1c,"Names Differ: %s %s",local_14 + 8,param_1);
      _fclose [GHIDRA-AUTO-LABEL: unreliable](local_1c);
    }
  }
  __security_check_cookie [GHIDRA-AUTO-LABEL: unreliable](local_20 ^ (uint)&stack0xfffffffc);
  return;
}
```

### decoy: 0x0048e130 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0048e130: 16-byte aligned: YES
  0048e130  55             PUSH EBP
  0048e131  8B EC          MOV EBP,ESP
  0048e133  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0048e130
string literals (0):
direct callees (1):
  0x0048E090  Global::BinaryInsertModel
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 309 B, RET 0
- callers 8: 0x0045a030 Gob::LoadAddInAnimations, 0x00462020 Gob::Append, 0x00462320, 0x0046f610, 0x00470290 Scene::SpawnRoom, ...
- callees: 0x009370c7 _stricmp (CALL), 0x0048e060 (CALL), 0x00511ed0 (CALL), 0x00511f50 (CALL), 0x00452830 (CALL), 0x0091caeb (CALL), ...
- strings: 0x0098d6a4 "Names Differ: %s %s", 0x0098d6b8 "AR_ERROR.LOG"
- twins: none

## Card B18-1-12

GOG function: Global::AurResGetNextLine  gog 0x00867ee0  callconv=(blank)  param_size_bytes=0  gog_size=69  gog_retn=0
Candidate Steam address: 0x00475fc0 (size 111, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.12 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00475f70:STR0/CALLEE4/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00867e10 Global::AurResFree  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00867fa0 Global::AurResFree_2 -> 0x00476030  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00867f30 unnamed -> Steam ?  in candidate callees: n/a
  0x00867fa0 Global::AurResFree_2 -> Steam 0x00476030  in candidate callees: no
GOG callers mapped through trusted rows (3 of 20): the card's A-CALLEE credit may come from these, not from callees
  0x00817210 CAurTextureBasic::Init -> Steam 0x00424b10  in candidate callers: no
  0x0084e530 Gob::EnableRenderBumpedOut -> Steam 0x0045bf50  in candidate callers: no
  0x00861e10 Scene::LoadVisibility -> Steam 0x0046fbd0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8,0x14}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x8,0xc,0xfa0}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00475fc0: 16-byte aligned: YES
  00475fc0  55             PUSH EBP
  00475fc1  8B EC          MOV EBP,ESP
  00475fc3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00475fc0
string literals (0):
direct callees (3):
  0x00475DF0  (unnamed)
  0x00919723  operator new
  0x0091F007  (unnamed)
struct offsets touched (top 3):
  +0x8 [dec 8]  x4  via EAX,ECX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x14 [dec 20]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00475fc0 FUN_00475fc0 size=111 ret=0 callers=2 callees=3

void FUN_00475fc0(undefined4 param_1)

{
  undefined4 *puVar1;
  undefined4 uVar2;
  
  puVar1 = (undefined4 *)FUN_00475df0(param_1);
  puVar1[1 = byte +0x4] = puVar1[1 = byte +0x4] + 1;
  if (puVar1[2 = byte +0x8] == 0) {
    uVar2 = FUN_00919723(puVar1[5 = byte +0x14]);
    puVar1[2 = byte +0x8] = uVar2;
    *(undefined1 *)puVar1[2 = byte +0x8] = 0;
  }
  _fgets [GHIDRA-AUTO-LABEL: unreliable]((char *)puVar1[2 = byte +0x8],4000,(FILE *)*puVar1);
  return;
}
```

### decoy: 0x00475f70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00475f70: 16-byte aligned: YES
  00475f70  55             PUSH EBP
  00475f71  8B EC          MOV EBP,ESP
  00475f73  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00475f70
string literals (0):
direct callees (2):
  0x00475FC0  (unnamed)
  0x00476030  Global::AurResFree_2
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 111 B, RET 0
- callers 2: 0x00475f70, 0x00476110
- callees: 0x00475df0 (CALL), 0x00919723 operator new (CALL), 0x0091f007 (CALL)
- twins: none

## Card B18-1-13

GOG function: CClientAIMaster::AddObject  gog 0x0044eeb0  callconv=(blank)  param_size_bytes=4  gog_size=36  gog_retn=4
Candidate Steam address: 0x007a2510 (size 36, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 4 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 4 (callers: HandleServerToPlayerPlaceableUpdate_Add 0x80d050, Constructor 0x86d770, AddToArea 0x800cb0)
- best decoy (neighbour/other candidate) scores: 0x007a2540:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044ed40 CClientAIMaster::UpdateState  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0044eee0 CClientAIMaster::RemoveObject  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044ead0 CClientAIList::AddObject -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (4 of 13): the card's A-CALLEE credit may come from these, not from callees
  0x004a1640 CSWCMessage::HandleServerToPlayerPlaceableUpdate_Add -> Steam 0x0080d050  in candidate callers: yes
  0x004f90c0 CSWCVisualEffect::Constructor -> Steam 0x0086d770  in candidate callers: yes
  0x0055b480 CSWCSoundObject::AddToArea -> Steam 0x00800cb0  in candidate callers: yes
  0x00578ec0 CSWCProjectile::Constructor -> Steam 0x008e54e0  in candidate callers: yes
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

## Card B18-1-14

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

## Card B18-1-15

GOG function: Global::InsertModel  gog 0x008ce680  callconv=(blank)  param_size_bytes=4  gog_size=48  gog_retn=0
Candidate Steam address: 0x0048e090 (size 148, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=Global::BinaryInsertModel | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0048e130:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x008ce5e0 Global::BinaryInsertModel -> 0x0048e090  trusted
- GOG-order neighbour after:  0x008ce6b0 Global::FindModel  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x008ce5e0 Global::BinaryInsertModel -> Steam 0x0048e090  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x8*,0xc*} | Steam {0x8*,0xc*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x0048e090: 16-byte aligned: YES
  0048e090  55             PUSH EBP
  0048e091  8B EC          MOV EBP,ESP
  0048e093  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0048e090
string literals (0):
direct callees (3):
  0x0050C380  (unnamed)
  0x009370C7  _stricmp
  0x0048E090  Global::BinaryInsertModel
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0048e090 FUN_0048e090 size=148 ret=0 callers=2 callees=3

void FUN_0048e090(int param_1,int param_2,int param_3)

{
  int iVar1;
  int iVar2;
  
  if (param_3 < param_2) {
    FUN_0050c380(param_1,param_2);
  }
  else {
    iVar1 = (param_2 + param_3) / 2;
    iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]((char *)(param_1 + 8),(char *)(*(int *)(DAT_00a1ffa8 + iVar1 * 4) + 8));
    if (iVar2 < 0) {
      FUN_0048e090(param_1,param_2,iVar1 + -1);
    }
    else {
      FUN_0048e090(param_1,iVar1 + 1,param_3);
    }
  }
  return;
}
```

### decoy: 0x0048e130 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0048e130: 16-byte aligned: YES
  0048e130  55             PUSH EBP
  0048e131  8B EC          MOV EBP,ESP
  0048e133  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0048e130
string literals (0):
direct callees (1):
  0x0048E090  Global::BinaryInsertModel
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 148 B, RET 0
- callers 2: 0x0048e090, 0x0048e130
- callees: 0x0050c380 (CALL), 0x009370c7 _stricmp (CALL), 0x0048e090 (CALL)
- twins: none

## Card B18-1-16

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
