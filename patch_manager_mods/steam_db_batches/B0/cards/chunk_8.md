# Review cards B0 chunk 8

Answer each card with one TSV row in `steam_db_batches/B0/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B0-8-01

GOG function: CSWSEffectListHandler::Destructor_2  gog 0x00736240  callconv=(blank)  param_size_bytes=0  gog_size=167  gog_retn=0
Candidate Steam address: 0x00593960 (size 167, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00593760:STR0/CALLEE0/SIG0.625/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00735c20 CGameEffect::GetEffectTypeScriptConstant -> 0x005e5f60  gate_class=AUTO
- GOG-order neighbour after:  0x007362f0 CSWSEffectListHandler::OnRemoveForceSight -> 0x00593a10  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
imm sets (* = in both, 2 common): GOG {0x4*,0xc*} | Steam {0x4*,0xc*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00593960: 16-byte aligned: YES
  00593960  55             PUSH EBP
  00593961  8B EC          MOV EBP,ESP
  00593963  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00593960
string literals (0):
direct callees (1):
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x4 [dec 4]  x3  via ECX,EDX
  +0x8 [dec 8]  x3  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00593960 FUN_00593960 size=167 ret=0 callers=1 callees=1

void __fastcall FUN_00593960(undefined4 *param_1)

{
  uint uVar1;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_00951c08;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *param_1 = CSWSEffectListHandler::vftable;
  local_8 = 0;
  if (param_1[1 = byte +0x4] != 0) {
    FUN_009196fd(param_1[1 = byte +0x4],uVar1);
    param_1[1 = byte +0x4] = 0;
  }
  if (param_1[2 = byte +0x8] != 0) {
    FUN_009196fd(param_1[2 = byte +0x8],uVar1);
    param_1[2 = byte +0x8] = 0;
  }
  *param_1 = CGameEffectApplierRemover::vftable;
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x00593760 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00593760: 16-byte aligned: YES
  00593760  55             PUSH EBP
  00593761  8B EC          MOV EBP,ESP
  00593763  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00593760
string literals (0):
direct callees (2):
  0x00919723  operator new
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x8 [dec 8]  x2  via EDX
  +0x4 [dec 4]  x1  via EAX
```

## Card B0-8-02

GOG function: CSWSArea::DetachFromClientArea  gog 0x00722d10  callconv=(blank)  param_size_bytes=0  gog_size=24  gog_retn=0
Candidate Steam address: 0x0052dc30 (size 24, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:batch5_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0052dc10:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00722cf0 CSWSArea::AttachmentFromClientArea -> 0x0052dc10  gate_class=REVIEW
- GOG-order neighbour after:  0x00722d30 CSWSArea::ClearObjects -> 0x0052dc50  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x334*} | Steam {0x334*}
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
prologue for 0x0052dc30: 16-byte aligned: YES
  0052dc30  55             PUSH EBP
  0052dc31  8B EC          MOV EBP,ESP
  0052dc33  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0052dc30
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x334 [dec 820]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0052dc30 FUN_0052dc30 size=24 ret=0 callers=1 callees=0

void __fastcall FUN_0052dc30(int param_1)

{
  *(undefined4 *)(param_1 + 0x334) = 0;
  return;
}
```

### decoy: 0x0052dc10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0052dc10: 16-byte aligned: YES
  0052dc10  55             PUSH EBP
  0052dc11  8B EC          MOV EBP,ESP
  0052dc13  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0052dc10
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x334 [dec 820]  x1  via EAX
```

## Card B0-8-03

GOG function: CSWSArea::SetMaxStealthXP  gog 0x007184d0  callconv=(blank)  param_size_bytes=0  gog_size=52  gog_retn=0
Candidate Steam address: 0x00521e60 (size 844, retn 0)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSArea::AIUpdate
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005219d0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00718460 CSWSArea::GetFirstObjectInArea -> 0x005232b0  gate_class=REVIEW
- GOG-order neighbour after:  0x00718510 CSWSArea::DecrementPlayersInArea -> 0x00523360  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00718550 CSWSArea::IncreaseAILevelPriority -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x12c} | Steam {0x4,0x8,0xa8,0xa9,0xaa,0x124,0x140,0x144,0x168,0x230,0x234,0x238,0x23c,0x240,...(+1)}
imm sets (* = in both, 0 common): GOG {} | Steam {0x2,0x3,0x4,0x2c,0x64,0xff,0x1770,0x493e0}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00521e60: 16-byte aligned: YES
  00521e60  55             PUSH EBP
  00521e61  8B EC          MOV EBP,ESP
  00521e63  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00521e60
string literals (0):
direct callees (6):
  0x0051C370  CServerExoApp::GetWorldTimer
  0x0051AD20  CWorldTimer::GetWorldTime
  0x0051AF60  CWorldTimer::SubtractWorldTimes
  0x0091EB8B  (unnamed)
  0x0052C820  CSWSArea::SetCurrentWeather
  0x006FD8D0  CVirtualMachine::RunScript
struct offsets touched (top 10):
  +0x234 [dec 564]  x13  via EAX,ECX,EDX
  +0x230 [dec 560]  x7  via EAX,ECX,EDX
  +0x238 [dec 568]  x6  via EAX,ECX
  +0x8 [dec 8]  x3  via EAX,ECX
  +0x144 [dec 324]  x3  via ECX,EDX
  +0xA8 [dec 168]  x2  via ECX,EDX
  +0xA9 [dec 169]  x2  via EAX,EDX
  +0xAA [dec 170]  x2  via EAX,ECX
  +0x140 [dec 320]  x2  via ECX,EDX
  +0x23C [dec 572]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00521e60 FUN_00521e60 size=844 ret=0 callers=1 callees=6

/* WARNING: Removing unreachable block (ram,0x00522165) */

undefined4 __fastcall FUN_00521e60(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  bool bVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 *puVar7;
  undefined1 *puVar8;
  undefined4 *puVar9;
  uint *puVar10;
  uint local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined1 local_8 [4];
  
  puVar9 = &local_c;
  puVar7 = &local_10;
  FUN_0051c370(puVar7,puVar9);
  FUN_0051ad20(puVar7,puVar9);
  puVar10 = &local_14;
  puVar8 = local_8;
  uVar2 = *(undefined4 *)(param_1 + 0x240);
  uVar6 = *(undefined4 *)(param_1 + 0x23c);
  uVar4 = local_10;
  uVar5 = local_c;
  FUN_0051c370(local_10,local_c,uVar6,uVar2,puVar8,puVar10);
  FUN_0051af60(uVar4,uVar5,uVar6,uVar2,puVar8,puVar10);
  *(undefined4 *)(param_1 + 0x23c) = local_10;
  *(undefined4 *)(param_1 + 0x240) = local_c;
  if ((*(char *)(param_1 + 0x244) == -1) && ((*(uint *)(param_1 + 4) & 1) == 0)) {
    if (*(int *)(param_1 + 0x238) == 0) {
      iVar1 = _rand [GHIDRA-AUTO-LABEL: unreliable]();
      iVar1 = iVar1 % 100;
      bVar3 = false;
      if (*(char *)(param_1 + 0xa8) == '\0') {
        if (*(char *)(param_1 + 0xa9) == '\0') {
          if (*(char *)(param_1 + 0xaa) != '\0') {
            if (iVar1 < (int)(uint)*(byte *)(param_1 + 0xaa)) {
              bVar3 = *(int *)(param_1 + 0x234) == 0;
              if (bVar3) {
                *(undefined1 *)(param_1 + 0x230) = 1;
                *(undefined4 *)(param_1 + 0x234) = 1;
              }
            }
            else if (*(int *)(param_1 + 0x234) != 0) {
              *(undefined1 *)(param_1 + 0x230) = 1;
              *(undefined4 *)(param_1 + 0x234) = 0;
              bVar3 = true;
            }
          }
        }
        else if (iVar1 < (int)(uint)*(byte *)(param_1 + 0xa9)) {
          bVar3 = *(int *)(param_1 + 0x234) == 0;
          if (bVar3) {
            *(undefined1 *)(param_1 + 0x230) = 2;
            *(undefined4 *)(param_1 + 0x234) = 1;
          }
        }
        else if (*(int *)(param_1 + 0x234) != 0) {
          *(undefined1 *)(param_1 + 0x230) = 2;
          *(undefined4 *)(param_1 + 0x234) = 0;
          bVar3 = true;
        }
      }
      else if (iVar1 < (int)(uint)*(byte *)(param_1 + 0xa8)) {
        bVar3 = *(int *)(param_1 + 0x234) == 0;
        if (bVar3) {
          *(undefined1 *)(param_1 + 0x230) = 1;
          *(undefined4 *)(param_1 + 0x234) = 1;
        }
      }
      else if (*(int *)(param_1 + 0x234) != 0) {
        *(undefined1 *)(param_1 + 0x230) = 1;
        *(undefined4 *)(param_1 + 0x234) = 0;
/* ... 35 more lines truncated by --max-lines */
```

### decoy: 0x005219d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005219d0: 16-byte aligned: YES
  005219d0  55             PUSH EBP
  005219d1  8B EC          MOV EBP,ESP
  005219d3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005219d0
string literals (0):
direct callees (19):
  0x0052B4E0  CSWSArea::UnloadArea
  0x0051C080  (unnamed)
  0x0053DEA0  (unnamed)
  0x006F9420  (unnamed)
  0x009196FD  operator delete
  0x0040C170  (unnamed)
  0x007AAEA0  CSWCArea::DetachFromServerArea
  0x0051BD90  GetModule (fastcall wrapper of 0x52F4C0)
  0x00557F80  (unnamed)
  0x0051CF00  (unnamed)
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x0040DF30  (unnamed)
  0x005E6DE0  CSWSScriptVarTable::Destructor
  0x00919BCC  `eh vector destructor iterator` (CRT)
  0x00733780  CExoString::~CExoString
  0x00735360  CExoLocString::Destructor
  0x0072F5F0  (unnamed)
  0x0052E590  (unnamed)
  0x005E7970  (unnamed)
struct offsets touched (top 10):
  +0x8 [dec 8]  x3  via EAX,EDX
  +0x1B4 [dec 436]  x3  via ECX,EDX
  +0x22C [dec 556]  x3  via EAX,ECX,EDX
  +0x260 [dec 608]  x3  via EAX,ECX
  +0x268 [dec 616]  x3  via EAX,ECX
  +0x280 [dec 640]  x3  via EAX,ECX
  +0x284 [dec 644]  x3  via EAX,EDX
  +0x288 [dec 648]  x3  via ECX,EDX
  +0x294 [dec 660]  x3  via EAX,ECX
  +0x298 [dec 664]  x3  via EAX,EDX
```

## Card B0-8-04

GOG function: CSWSEffectListHandler::OnRemoveTemporaryHitpoints  gog 0x0073d5b0  callconv=(blank)  param_size_bytes=8  gog_size=1121  gog_retn=8
Candidate Steam address: 0x0059acc0 (size 1121, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0059ac50:STR0/CALLEE0/SIG0.083/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0073d540 CSWSEffectListHandler::OnApplyTemporaryHitpoints -> 0x0059ac50  gate_class=REVIEW
- GOG-order neighbour after:  0x0073da20 CSWSEffectListHandler::OnApplyDamageImmunityIncrease -> 0x0059b130  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (17), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00605b70 CExoString::operator==_2 -> Steam ?  in candidate callees: n/a
  0x00620c70 C2DA::GetCExoStringEntry_3 -> Steam ?  in candidate callees: n/a
  0x0064b7b0 CServerExoApp::GetModule -> Steam ?  in candidate callees: n/a
  0x0064bca0 CServerExoApp::GetServerAIMaster -> Steam ?  in candidate callees: n/a
  0x0064db90 CServerAIMaster::AddEventDeltaTime -> Steam ?  in candidate callees: n/a
  ... 9 more GOG callees
GOG strings (1): 'BLOODCOLR'
disp sets (* = in both, 12 common): GOG {0x4*,0x8*,0xa*,0x10*,0x30*,0x94*,0x9c*,0xe8*,0xec*,0x130*,0x1184*,0x11ac*} | Steam {0x4*,0x8*,0xa*,0x10*,0x30*,0x94*,0x9c*,0xe8*,0xec*,0x130*,0x1184*,0x11ac*}
imm sets (* = in both, 16 common): GOG {0x2*,0x3*,0x4*,0x8*,0x13*,0x1c*,0x1e*,0x20*,0x34*,0x84*,0x8c*,0x9e*,0x9f*,0xa0*,...(+2)} | Steam {0x2*,0x3*,0x4*,0x8*,0x13*,0x1c*,0x1e*,0x20*,0x34*,0x84*,0x8c*,0x9e*,0x9f*,0xa0*,...(+2)}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0059acc0: 16-byte aligned: YES
  0059acc0  55             PUSH EBP
  0059acc1  8B EC          MOV EBP,ESP
  0059acc3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0059acc0
string literals (1):
  0x00996328  at 0059afaa  'BLOODCOLR'
direct callees (17):
  0x005E4E60  CGameEffect::GetInteger
  0x0057A990  CSWSCreature::RecomputeAmbientAnimationState
  0x0051BD90  GetModule (fastcall wrapper of 0x52F4C0)
  0x00919723  operator new
  0x005E1A60  (unnamed)
  0x0051C280  (unnamed)
  0x0051E170  CServerAIMaster::AddEventDeltaTime(dayD,msD,callerId,targetId,eventId,data)
  0x00584EF0  (unnamed)
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x00733540  CExoString::CExoString(empty)
  0x00733570  CExoString::CExoString_char
  0x0071B1D0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x00733A60  CExoString::operator==_2
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 10):
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0xA [dec 10]  x4  via ECX,EDX
  +0xE8 [dec 232]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX
  +0x30 [dec 48]  x2  via EDX
  +0x94 [dec 148]  x2  via EAX,EDX
  +0x9C [dec 156]  x2  via EAX,EDX
  +0x11AC [dec 4524]  x2  via EAX,ECX
  +0x10 [dec 16]  x1  via EDX
  +0xEC [dec 236]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0059acc0 FUN_0059acc0 size=1121 ret=8 callers=0 callees=17

/* WARNING: Removing unreachable block (ram,0x0059add5) */
/* WARNING: Removing unreachable block (ram,0x0059ae37) */
/* WARNING: Removing unreachable block (ram,0x0059ae27) */
/* WARNING: Removing unreachable block (ram,0x0059ae41) */

undefined4 FUN_0059acc0(int *param_1,int param_2)

{
  bool bVar1;
  short sVar2;
  int iVar3;
  int iVar4;
  int local_94;
  int local_90;
  int local_60;
  undefined1 local_50 [8];
  int local_48;
  int local_44;
  undefined1 local_38 [8];
  int local_30;
  int local_2c;
  int *local_20;
  uint local_1c;
  undefined4 local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952011;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar3 = (**(code **)(*param_1 + 0x30))(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  if (iVar3 == 0) {
    ExceptionList = local_10;
    return 1;
  }
  local_20 = (int *)(**(code **)(*param_1 + 0x30))();
  local_18 = 0;
  local_14 = (**(code **)(*local_20 + 0x94))();
  if (local_20[0x46b = byte +0x11ac] == 0) {
    local_1c = 0;
  }
  else {
    sVar2 = (**(code **)(*local_20 + 0x9c))(0);
    local_1c = (uint)(sVar2 < 1);
  }
  if (param_2 != 0) {
    iVar3 = local_20[0x3a = byte +0xe8];
    iVar4 = FUN_005e4e60(0);
    if ((short)iVar3 == iVar4 || (short)iVar3 - iVar4 < 0) {
      local_60 = 0;
    }
    else {
      iVar3 = local_20[0x3a = byte +0xe8];
      local_60 = FUN_005e4e60(0);
      local_60 = (short)iVar3 - local_60;
    }
    local_20[0x3a = byte +0xe8] = local_60;
  }
  if ((local_14 != 0) || (iVar3 = (**(code **)(*local_20 + 0x94))(), iVar3 != 1)) {
    if (local_1c != 0) {
      ExceptionList = local_10;
      return 1;
    }
    if (local_20[0x46b = byte +0x11ac] == 0) {
      bVar1 = false;
    }
    else {
      sVar2 = (**(code **)(*local_20 + 0x9c))(0);
      bVar1 = sVar2 < 1;
    }
    if (!bVar1) {
      ExceptionList = local_10;
      return 1;
    }
  }
  local_48 = FUN_00919723(0x8c);
/* ... 56 more lines truncated by --max-lines */
```

### decoy: 0x0059ac50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059ac50: 16-byte aligned: YES
  0059ac50  55             PUSH EBP
  0059ac51  8B EC          MOV EBP,ESP
  0059ac53  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059ac50
string literals (0):
direct callees (1):
  0x005E4E60  CGameEffect::GetInteger
struct offsets touched (top 2):
  +0x30 [dec 48]  x2  via EDX
  +0xE8 [dec 232]  x2  via ECX,EDX
```

## Card B0-8-05

GOG function: CSWSEffectListHandler::OnApplyDamageIncrease  gog 0x0073d410  callconv=(blank)  param_size_bytes=4  gog_size=63  gog_retn=12
Candidate Steam address: 0x0059ab70 (size 63, retn 12)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0059aa40:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0073d330 CSWSEffectListHandler::OnApplyAttackDecrease -> 0x0059aa90  gate_class=REVIEW
- GOG-order neighbour after:  0x0073d450 CSWSEffectListHandler::OnApplyDamageDecrease -> 0x0059abb0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x30*,0x384*} | Steam {0x30*,0x384*}
imm sets (* = in both, 2 common): GOG {0x8*,0xc*} | Steam {0x8*,0xc*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 12)
```

### prologue
```
prologue for 0x0059ab70: 16-byte aligned: YES
  0059ab70  55             PUSH EBP
  0059ab71  8B EC          MOV EBP,ESP
  0059ab73  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059ab70
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x30 [dec 48]  x2  via EDX
  +0x384 [dec 900]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0059ab70 FUN_0059ab70 size=63 ret=12 callers=0 callees=0

undefined4 __thiscall FUN_0059ab70(undefined4 param_1,int *param_2)

{
  int iVar1;
  
  iVar1 = (**(code **)(*param_2 + 0x30))(param_1);
  if (iVar1 != 0) {
    iVar1 = (**(code **)(*param_2 + 0x30))();
    *(undefined4 *)(iVar1 + 0x384 [dec 900]) = 1;
  }
  return 0;
}
```

### decoy: 0x0059aa40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059aa40: 16-byte aligned: YES
  0059aa40  55             PUSH EBP
  0059aa41  8B EC          MOV EBP,ESP
  0059aa43  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059aa40
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x30 [dec 48]  x2  via EDX
  +0x384 [dec 900]  x1  via ECX
```

## Card B0-8-06

GOG function: CGameEffect::SetExpiryTime  gog 0x00734c40  callconv=(blank)  param_size_bytes=8  gog_size=31  gog_retn=8
Candidate Steam address: 0x005e4f80 (size 31, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005e4fa0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00734c20 CGameEffect::SetString -> 0x005e4f60  gate_class=REVIEW
- GOG-order neighbour after:  0x00734c60 CGameEffect::GetExpiryTime -> 0x005e4fa0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x10*,0x14*} | Steam {0x10*,0x14*}
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
prologue for 0x005e4f80: 16-byte aligned: YES
  005e4f80  55             PUSH EBP
  005e4f81  8B EC          MOV EBP,ESP
  005e4f83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005e4f80
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x10 [dec 16]  x1  via EAX
  +0x14 [dec 20]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005e4f80 FUN_005e4f80 size=31 ret=8 callers=5 callees=0

void __thiscall FUN_005e4f80(int param_1,undefined4 param_2,undefined4 param_3)

{
  *(undefined4 *)(param_1 + 0x10) = param_2;
  *(undefined4 *)(param_1 + 0x14) = param_3;
  return;
}
```

### decoy: 0x005e4fa0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005e4fa0: 16-byte aligned: YES
  005e4fa0  55             PUSH EBP
  005e4fa1  8B EC          MOV EBP,ESP
  005e4fa3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005e4fa0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x10 [dec 16]  x1  via ECX
  +0x14 [dec 20]  x1  via ECX
```

## Card B0-8-07

GOG function: CSWSEffectListHandler::OnRemoveDamageImmunityDecrease  gog 0x0073dcd0  callconv=(blank)  param_size_bytes=8  gog_size=313  gog_retn=8
Candidate Steam address: 0x0059b3e0 (size 313, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0059b1a0:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0073dbd0 CSWSEffectListHandler::OnApplyDamageImmunityDecrease -> 0x0059b2e0  gate_class=REVIEW
- GOG-order neighbour after:  0x0073de10 CSWSEffectListHandler::OnApplyEntangled -> 0x0059b520  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006a9ed0 CSWSObject::SetDamageImmunity -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0xb0*} | Steam {0x4*,0x8*,0xb0*}
imm sets (* = in both, 5 common): GOG {0x8*,0x10*,0x11*,0x2c*,0x148*} | Steam {0x8*,0x10*,0x11*,0x2c*,0x148*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0059b3e0: 16-byte aligned: YES
  0059b3e0  55             PUSH EBP
  0059b3e1  8B EC          MOV EBP,ESP
  0059b3e3  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059b3e0
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x005431F0  (unnamed)
struct offsets touched (top 3):
  +0x8 [dec 8]  x3  via EDX
  +0x4 [dec 4]  x1  via ECX
  +0xB0 [dec 176]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0059b3e0 FUN_0059b3e0 size=313 ret=8 callers=0 callees=2

undefined4 FUN_0059b3e0(int *param_1,int param_2)

{
  int iVar1;
  char cVar2;
  char cVar3;
  char cVar4;
  uint uVar5;
  uint uVar6;
  undefined4 extraout_var;
  undefined4 uVar7;
  int local_8;
  
  if (param_2 != 0) {
    uVar5 = FUN_005e4e60(0);
    uVar7 = extraout_var;
    cVar2 = FUN_005e4e60(1);
    (**(code **)(*param_1 + 0xb0))(uVar5 & 0xff,uVar7);
    cVar4 = '\0';
    for (local_8 = 0; local_8 < param_1[0x53 = byte +0x14c]; local_8 = local_8 + 1) {
      iVar1 = *(int *)(param_1[0x52 = byte +0x148] + local_8 * 4);
      if (*(short *)(iVar1 + 8) == 0x10) {
        uVar6 = FUN_005e4e60(0);
        if (uVar6 != uVar5) goto LAB_0059b493;
LAB_0059b4b6:
        if (*(short *)(iVar1 + 8) == 0x10) {
          cVar3 = FUN_005e4e60(1);
          cVar4 = cVar3 + cVar4;
        }
        else {
          cVar3 = FUN_005e4e60(1);
          cVar4 = cVar4 - cVar3;
        }
      }
      else {
LAB_0059b493:
        if (*(short *)(iVar1 + 8) == 0x11) {
          uVar6 = FUN_005e4e60(0);
          if (uVar6 == uVar5) goto LAB_0059b4b6;
        }
      }
    }
    FUN_005431f0(uVar5 & 0xffff,cVar4 - cVar2);
  }
  return 1;
}
```

### decoy: 0x0059b1a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059b1a0: 16-byte aligned: YES
  0059b1a0  55             PUSH EBP
  0059b1a1  8B EC          MOV EBP,ESP
  0059b1a3  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059b1a0
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x005431F0  (unnamed)
struct offsets touched (top 3):
  +0x8 [dec 8]  x3  via EDX
  +0x4 [dec 4]  x1  via ECX
  +0xB4 [dec 180]  x1  via EDX
```

## Card B0-8-08

GOG function: CGameEffect::SetObjectID  gog 0x00734bc0  callconv=(blank)  param_size_bytes=8  gog_size=26  gog_retn=8
Candidate Steam address: 0x005e4f00 (size 26, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005e4ee0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00734ba0 CGameEffect::GetObjectID -> 0x005e4ee0  gate_class=REVIEW
- GOG-order neighbour after:  0x00734be0 CGameEffect::GetString -> 0x005e4f20  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x78*} | Steam {0x78*}
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
prologue for 0x005e4f00: 16-byte aligned: YES
  005e4f00  55             PUSH EBP
  005e4f01  8B EC          MOV EBP,ESP
  005e4f03  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005e4f00
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005e4f00 FUN_005e4f00 size=26 ret=8 callers=17 callees=0

void __thiscall FUN_005e4f00(int param_1,int param_2,undefined4 param_3)

{
  *(undefined4 *)(param_1 + 0x78 + param_2 * 4) = param_3;
  return;
}
```

### decoy: 0x005e4ee0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005e4ee0: 16-byte aligned: YES
  005e4ee0  55             PUSH EBP
  005e4ee1  8B EC          MOV EBP,ESP
  005e4ee3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005e4ee0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

## Card B0-8-09

GOG function: CSWSArea::GetNumMinesSetByPlayerParty  gog 0x007232a0  callconv=(blank)  param_size_bytes=0  gog_size=161  gog_retn=0
Candidate Steam address: 0x0052e1c0 (size 161, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:batch5_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0052e170:STR0/CALLEE0/SIG0.083/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00723250 CSWSArea::SetPlayerRestrictMode -> 0x0052e170  gate_class=REVIEW
- GOG-order neighbour after:  0x00723350 CSWSArea::GetPlayerCanSetMines -> 0x0052e270  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0064bb60 CServerExoApp::GetTriggerByGameObjectID -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x8*,0x130*,0x134*,0x2fc*,0x354*} | Steam {0x8*,0x130*,0x134*,0x2fc*,0x354*}
imm sets (* = in both, 1 common): GOG {0x24*} | Steam {0x24*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0052e1c0: 16-byte aligned: YES
  0052e1c0  55             PUSH EBP
  0052e1c1  8B EC          MOV EBP,ESP
  0052e1c3  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0052e1c0
string literals (0):
direct callees (1):
  0x0051C140  (unnamed)
struct offsets touched (top 5):
  +0x8 [dec 8]  x1  via EDX
  +0x130 [dec 304]  x1  via ECX
  +0x134 [dec 308]  x1  via ECX
  +0x2FC [dec 764]  x1  via EAX
  +0x354 [dec 852]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0052e1c0 FUN_0052e1c0 size=161 ret=0 callers=1 callees=1

undefined4 __fastcall FUN_0052e1c0(int param_1)

{
  int iVar1;
  undefined4 local_c;
  undefined1 local_5;
  
  local_5 = '\0';
  local_c = 0;
  while( true ) {
    if (*(int *)(param_1 + 0x134) <= local_c) break;
    iVar1 = FUN_0051c140(*(undefined4 *)(*(int *)(param_1 + 0x130) + local_c * 4));
    if (((iVar1 != 0) && (*(int *)(iVar1 + 0x2fc) != 0)) && (*(int *)(iVar1 + 0x354) != 0)) {
      local_5 = local_5 + '\x01';
    }
    local_c = local_c + 1;
  }
  return CONCAT31((int3)((uint)local_c >> 8),local_5);
}
```

### decoy: 0x0052e170 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0052e170: 16-byte aligned: YES
  0052e170  55             PUSH EBP
  0052e171  8B EC          MOV EBP,ESP
  0052e173  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0052e170
string literals (0):
direct callees (2):
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005FEF20  CSWPartyTable::UnstealthParty
struct offsets touched (top 2):
  +0x2E4 [dec 740]  x2  via EAX,ECX
  +0x8 [dec 8]  x1  via EDX
```

## Card B0-8-10

GOG function: CSWSEffectListHandler::OnApplySavingThrowIncrease  gog 0x0073c6d0  callconv=(blank)  param_size_bytes=8  gog_size=95  gog_retn=12
Candidate Steam address: 0x00599de0 (size 95, retn 12)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00599e40:STR0/CALLEE0/SIG0.333/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0073c680 CSWSEffectListHandler::OnRemoveSummonParty -> 0x00599d90  gate_class=REVIEW
- GOG-order neighbour after:  0x0073c730 CSWSEffectListHandler::OnApplySavingThrowDecrease -> 0x00599e40  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x30*,0x384*} | Steam {0x30*,0x384*}
imm sets (* = in both, 1 common): GOG {0xc*} | Steam {0xc*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x00599de0: 16-byte aligned: YES
  00599de0  55             PUSH EBP
  00599de1  8B EC          MOV EBP,ESP
  00599de3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00599de0
string literals (0):
direct callees (1):
  0x005E4E60  CGameEffect::GetInteger
struct offsets touched (top 2):
  +0x30 [dec 48]  x2  via EDX
  +0x384 [dec 900]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00599de0 FUN_00599de0 size=95 ret=12 callers=0 callees=1

undefined4 __thiscall FUN_00599de0(undefined4 param_1,int *param_2,int param_3)

{
  int iVar1;
  int iVar2;
  
  iVar1 = (**(code **)(*param_2 + 0x30))(param_1);
  if ((iVar1 != 0) && (iVar1 = (**(code **)(*param_2 + 0x30))(), param_3 != 0)) {
    iVar2 = FUN_005e4e60(0);
    if (iVar2 < 1) {
      return 1;
    }
    *(undefined4 *)(iVar1 + 0x384 [dec 900]) = 1;
  }
  return 0;
}
```

### decoy: 0x00599e40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00599e40: 16-byte aligned: YES
  00599e40  55             PUSH EBP
  00599e41  8B EC          MOV EBP,ESP
  00599e43  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00599e40
string literals (0):
direct callees (3):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006BC570  CSWSCreatureStats::GetEffectImmunity
  0x005E4E60  CGameEffect::GetInteger
struct offsets touched (top 6):
  +0x30 [dec 48]  x2  via EDX
  +0x8 [dec 8]  x1  via EAX
  +0x18 [dec 24]  x1  via ECX
  +0xFC [dec 252]  x1  via EAX
  +0x384 [dec 900]  x1  via EDX
  +0x1198 [dec 4504]  x1  via EAX
```

## Card B0-8-11

GOG function: CGameEffect::GetString  gog 0x00734be0  callconv=(blank)  param_size_bytes=8  gog_size=53  gog_retn=8
Candidate Steam address: 0x005e4f20 (size 53, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005e4f60:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00734bc0 CGameEffect::SetObjectID -> 0x005e4f00  gate_class=REVIEW
- GOG-order neighbour after:  0x00734c20 CGameEffect::SetString -> 0x005e4f60  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00605700 CExoString::Constructor_3 -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x48*} | Steam {0x48*}
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
prologue for 0x005e4f20: 16-byte aligned: YES
  005e4f20  55             PUSH EBP
  005e4f21  8B EC          MOV EBP,ESP
  005e4f23  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005e4f20
string literals (0):
direct callees (1):
  0x007335F0  CExoString::CExoString_copy
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005e4f20 FUN_005e4f20 size=53 ret=8 callers=6 callees=1

undefined4 __thiscall FUN_005e4f20(int param_1,undefined4 param_2,int param_3)

{
  FUN_007335f0(param_1 + 0x48 + param_3 * 8);
  return param_2;
}
```

### decoy: 0x005e4f60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005e4f60: 16-byte aligned: YES
  005e4f60  55             PUSH EBP
  005e4f61  8B EC          MOV EBP,ESP
  005e4f63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005e4f60
string literals (0):
direct callees (1):
  0x007337C0  CExoString::operator=
struct offsets touched (top 0):
```

## Card B0-8-12

GOG function: CVirtualMachineInternal::RunScriptFile  gog 0x00645af0  callconv=(blank)  param_size_bytes=4  gog_size=345  gog_retn=4
Candidate Steam address: 0x00701fd0 (size 716, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CVirtualMachineInternal::ReadScriptFile
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.09 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007022a0:STR0/CALLEE0/SIG0.262/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00645830 CVirtualMachineInternal::RunScriptSituation -> 0x00702590  gate_class=REVIEW
- GOG-order neighbour after:  0x00645c50 CVirtualMachineInternal::GetRunScriptReturnValue -> 0x007029b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  0x00642680 CVirtualMachineStack::GetStackPointer -> Steam ?  in candidate callees: n/a
  0x00642690 CVirtualMachineStack::SetStackPointer -> Steam ?  in candidate callees: n/a
  0x00642c30 CVirtualMachineInternal::ExecuteCode -> Steam ?  in candidate callees: n/a
  0x00645220 CVirtualMachineInternal::PushInstructionPtr -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x10,0x1c,0x20,0x28*,0x3c,0x40,0x44*,0x1ac,0x1c4,0x3d4} | Steam {0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x28*,0x2c,0x44*}
imm sets (* = in both, 2 common): GOG {0x4*,0x1c,0x28*,0x1ac} | Steam {0x4*,0x8,0x9,0xa,0x10,0x18,0x20,0x28*,0x2e,0x30,0x31,0x39,0x42,0x43,...(+11)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00701fd0: 16-byte aligned: YES
  00701fd0  55             PUSH EBP
  00701fd1  8B EC          MOV EBP,ESP
  00701fd3  83 EC 44       SUB ESP,0x44
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00701fd0
string literals (0):
direct callees (9):
  0x00710840  (unnamed)
  0x00703950  (unnamed)
  0x00710810  (unnamed)
  0x007111E0  CRes::Demand
  0x006F3D90  (unnamed)
  0x00703BD0  (unnamed)
  0x007337C0  CExoString::operator=
  0x007112B0  CRes::Release
  0x006FF8A0  CVirtualMachineInternal::InitializeScript
struct offsets touched (top 9):
  +0x28 [dec 40]  x15  via EAX,ECX,EDX
  +0x8 [dec 8]  x8  via EAX,ECX,EDX
  +0x5 [dec 5]  x3  via EAX,ECX,EDX
  +0x7 [dec 7]  x3  via EAX,ECX,EDX
  +0x1 [dec 1]  x2  via EAX,ECX
  +0x2 [dec 2]  x2  via EAX,EDX
  +0x3 [dec 3]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via ECX
  +0x6 [dec 6]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00701fd0 FUN_00701fd0 size=716 ret=4 callers=3 callees=9

undefined4 __thiscall FUN_00701fd0(int param_1,int param_2)

{
  undefined4 uVar1;
  int iVar2;
  undefined1 local_44 [16];
  undefined1 local_34 [16];
  undefined1 local_24 [16];
  int local_14;
  int local_10;
  int local_c;
  char *local_8;
  
  *(int *)(param_1 + 0x28) = *(int *)(param_1 + 0x28) + 1;
  if (*(int *)(param_1 + 0x28) < 8) {
    if (param_2 == 0) {
      FUN_00710840(&DAT_00986925);
      FUN_00703950(local_24,0);
    }
    else {
      FUN_00710810(param_2);
      FUN_00703950(local_34,0);
    }
    iVar2 = FUN_007111e0();
    if (iVar2 == 0) {
      *(int *)(param_1 + 0x28) = *(int *)(param_1 + 0x28) + -1;
      uVar1 = 0xffffffa1;
    }
    else {
      local_8 = (char *)FUN_006f3d90();
      local_c = FUN_00703bd0();
      FUN_007337c0(param_2);
      if (((((*local_8 == 'N') && (local_8[1] == 'C')) && (local_8[2] == 'S')) &&
          ((local_8[3] == ' ' && (local_8[4] == 'V')))) && (local_8[6] == '.')) {
        local_10 = 0;
        if (('0' < local_8[5]) && (local_8[5] < ':')) {
          local_10 = (local_8[5] + -0x30) * 10;
        }
        if (('0' < local_8[7]) && (local_8[7] < ':')) {
          local_10 = local_10 + -0x30 + (int)local_8[7];
        }
        if (local_10 == 10) {
          if (local_8[8] == 'B') {
            local_14 = (uint)(byte)local_8[0xc] +
                       (uint)(byte)local_8[0xb] * 0x100 +
                       (uint)(byte)local_8[10] * 0x10000 + local_8[9] * 0x1000000;
            local_8 = local_8 + 0xd;
            local_c = local_c + -0xd;
            FUN_006ff8a0(param_1 + 0x2c + *(int *)(param_1 + 0x28) * 0x28,local_8,local_c);
            FUN_007112b0();
            FUN_00710840(&DAT_00986925);
            FUN_00703950(local_44,0);
            uVar1 = 0;
          }
          else {
            *(int *)(param_1 + 0x28) = *(int *)(param_1 + 0x28) + -1;
            FUN_007112b0();
            uVar1 = 0xffffffa0;
          }
        }
        else {
          *(int *)(param_1 + 0x28) = *(int *)(param_1 + 0x28) + -1;
          FUN_007112b0();
          uVar1 = 0xffffffa0;
        }
      }
      else {
        *(int *)(param_1 + 0x28) = *(int *)(param_1 + 0x28) + -1;
        FUN_007112b0();
        uVar1 = 0xffffffa0;
      }
    }
  }
  else {
    *(int *)(param_1 + 0x28) = *(int *)(param_1 + 0x28) + -1;
    uVar1 = 0xffffffa2;
  }
  return uVar1;
}
```

### decoy: 0x007022a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007022a0: 16-byte aligned: YES
  007022a0  55             PUSH EBP
  007022a1  8B EC          MOV EBP,ESP
  007022a3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007022a0
string literals (0):
direct callees (7):
  0x006FE440  CVirtualMachineStack::Destructor
  0x00701FD0  CVirtualMachineInternal::ReadScriptFile
  0x006FF440  CVirtualMachineStack::GetStackPointer
  0x00702850  CVirtualMachineInternal::RunScriptFile
  0x006FF930  CVirtualMachineInternal::DeleteScript
  0x00734010  CExoString::CStr
  0x006FF450  CVirtualMachineStack::SetStackPointer
struct offsets touched (top 10):
  +0x28 [dec 40]  x18  via EAX,ECX,EDX
  +0x3D4 [dec 980]  x8  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0xC [dec 12]  x1  via EDX
  +0x1C [dec 28]  x1  via EAX
  +0x20 [dec 32]  x1  via EAX
  +0x24 [dec 36]  x1  via ECX
  +0x1B8 [dec 440]  x1  via EDX
  +0x1BC [dec 444]  x1  via ECX
```

## Card B0-8-13

GOG function: CSWSArea::GetPlayerCanSetMines  gog 0x00723350  callconv=(blank)  param_size_bytes=0  gog_size=32  gog_retn=0
Candidate Steam address: 0x0052e270 (size 32, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:batch5_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0052e1c0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007232a0 CSWSArea::GetNumMinesSetByPlayerParty -> 0x0052e1c0  gate_class=REVIEW
- GOG-order neighbour after:  0x00723410 DirtyTextureController::Init  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007232a0 CSWSArea::GetNumMinesSetByPlayerParty -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0xf*} | Steam {0xf*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0052e270: 16-byte aligned: YES
  0052e270  55             PUSH EBP
  0052e271  8B EC          MOV EBP,ESP
  0052e273  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0052e270
string literals (0):
direct callees (1):
  0x0052E1C0  CSWSArea::GetNumMinesSetByPlayerParty
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0052e270 FUN_0052e270 size=32 ret=0 callers=3 callees=1

bool __fastcall FUN_0052e270(undefined4 param_1)

{
  byte bVar1;
  
  bVar1 = FUN_0052e1c0(param_1);
  return bVar1 < 0xf;
}
```

### decoy: 0x0052e1c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0052e1c0: 16-byte aligned: YES
  0052e1c0  55             PUSH EBP
  0052e1c1  8B EC          MOV EBP,ESP
  0052e1c3  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0052e1c0
string literals (0):
direct callees (1):
  0x0051C140  (unnamed)
struct offsets touched (top 5):
  +0x8 [dec 8]  x1  via EDX
  +0x130 [dec 304]  x1  via ECX
  +0x134 [dec 308]  x1  via ECX
  +0x2FC [dec 764]  x1  via EAX
  +0x354 [dec 852]  x1  via EDX
```

## Card B0-8-14

GOG function: CSWSEffectListHandler::OnApplyDamageDecrease  gog 0x0073d450  callconv=(blank)  param_size_bytes=8  gog_size=151  gog_retn=12
Candidate Steam address: 0x0059abb0 (size 151, retn 12)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0059aa90:STR0/CALLEE0/SIG0.517/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0073d410 CSWSEffectListHandler::OnApplyDamageIncrease -> 0x0059ab70  gate_class=REVIEW
- GOG-order neighbour after:  0x0073d4f0 CSWSEffectListHandler::OnRemoveDamageDecrease -> 0x0059aa40  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x006f89f0 CSWSCreatureStats::GetEffectImmunity -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x8*,0x18*,0x30*,0x384*,0x1198*} | Steam {0x8*,0x18*,0x30*,0x384*,0x1198*}
imm sets (* = in both, 3 common): GOG {0xc*,0x15*,0x1c*} | Steam {0xc*,0x15*,0x1c*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x0059abb0: 16-byte aligned: YES
  0059abb0  55             PUSH EBP
  0059abb1  8B EC          MOV EBP,ESP
  0059abb3  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059abb0
string literals (0):
direct callees (2):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006BC570  CSWSCreatureStats::GetEffectImmunity
struct offsets touched (top 5):
  +0x30 [dec 48]  x3  via EDX
  +0x8 [dec 8]  x1  via EAX
  +0x18 [dec 24]  x1  via ECX
  +0x384 [dec 900]  x1  via ECX
  +0x1198 [dec 4504]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0059abb0 FUN_0059abb0 size=151 ret=12 callers=0 callees=2

undefined4 __thiscall FUN_0059abb0(undefined4 param_1,int *param_2,int param_3)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = (**(code **)(*param_2 + 0x30))(param_1);
  if (iVar1 != 0) {
    uVar2 = FUN_0051c100(*(undefined4 *)(param_3 + 0x18));
    (**(code **)(*param_2 + 0x30))();
    iVar1 = FUN_006bc570(0x15,uVar2);
    if (iVar1 != 0) {
      return 1;
    }
    iVar1 = (**(code **)(*param_2 + 0x30))();
    *(undefined4 *)(iVar1 + 0x384 [dec 900]) = 1;
  }
  return 0;
}
```

### decoy: 0x0059aa90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059aa90: 16-byte aligned: YES
  0059aa90  55             PUSH EBP
  0059aa91  8B EC          MOV EBP,ESP
  0059aa93  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059aa90
string literals (0):
direct callees (3):
  0x005E4E60  CGameEffect::GetInteger
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006BC570  CSWSCreatureStats::GetEffectImmunity
struct offsets touched (top 6):
  +0x30 [dec 48]  x3  via EAX,EDX
  +0x8 [dec 8]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
  +0xFC [dec 252]  x1  via ECX
  +0x384 [dec 900]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EAX
```

## Card B0-8-15

GOG function: CSWSItemPropertyHandler::ApplyACBonus  gog 0x007c53c0  callconv=__thiscall  param_size_bytes=12  gog_size=686  gog_retn=20
Candidate Steam address: 0x005adaf0 (size 479, retn 20)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSItemPropertyHandler::ApplyAbilityBonus
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.58 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.58
- best decoy (neighbour/other candidate) scores: 0x005adcf0:STR0/CALLEE0/SIG0.256/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c5100 CSWSItemPropertyHandler::RemoveAbilityBonus -> 0x005adcf0  gate_class=REVIEW
- GOG-order neighbour after:  0x007c5670 CSWSItemPropertyHandler::RemoveACBonus -> 0x005ae260  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (12), mapped to Steam through trusted rows only:
  0x00489130 CTwoDimArrays::GetIPRPCostTable -> Steam ?  in candidate callees: n/a
  0x004eafd0 CSWBaseItemArray::GetBaseItem -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734960 CGameEffect::Destructor -> Steam ?  in candidate callees: n/a
  ... 4 more GOG callees
GOG strings (1): 'Value'
disp sets (* = in both, 6 common): GOG {0x2*,0x6*,0x8*,0xa*,0xc,0x14*,0x34,0x8b,0x122,0x130*} | Steam {0x2*,0x6*,0x8*,0xa*,0x14*,0x130*}
imm sets (* = in both, 5 common): GOG {0x2,0x3,0x4*,0x5*,0x14*,0x64,0x8c*,0xfffffff8*} | Steam {0x4*,0x5*,0x14*,0x3c,0x8c*,0xfffffff8*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 20)
```

### prologue
```
prologue for 0x005adaf0: 16-byte aligned: YES
  005adaf0  55             PUSH EBP
  005adaf1  8B EC          MOV EBP,ESP
  005adaf3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005adaf0
string literals (1):
  0x00992E8C  at 005adb18  'Value'
direct callees (9):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 6):
  +0xA [dec 10]  x2  via ECX
  +0x2 [dec 2]  x1  via EDX
  +0x6 [dec 6]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
  +0x14 [dec 20]  x1  via EDX
  +0x130 [dec 304]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005adaf0 FUN_005adaf0 size=479 ret=20 callers=0 callees=9

undefined4
FUN_005adaf0(int param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  undefined1 *puVar4;
  int *piVar5;
  int local_48;
  undefined2 local_34;
  undefined1 local_20 [8];
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952e03;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570("Value");
  local_8 = 0;
  piVar5 = &local_14;
  puVar4 = local_20;
  uVar2 = (uint)*(ushort *)(param_2 + 6);
  FUN_006f0190(1);
  FUN_0071bcf0(uVar2,puVar4,piVar5);
  local_8 = 0xffffffff;
  FUN_00733780(uVar1);
  if (local_14 != 0) {
    iVar3 = FUN_00919723(0x8c);
    local_8 = 1;
    if (iVar3 == 0) {
      local_48 = 0;
    }
    else {
      local_48 = FUN_005e4920(1);
    }
    local_8 = 0xffffffff;
    local_18 = local_48;
    local_34 = (undefined2)DAT_00995fac;
    *(undefined2 *)(local_48 + 8) = local_34;
    *(ushort *)(local_48 + 0xa [dec 10]) = *(ushort *)(local_48 + 0xa [dec 10]) & 0xfff8 | DAT_009967b0;
    FUN_005e5ea0(*(undefined4 *)(param_1 + 0x14));
    switch(*(undefined2 *)(param_2 + 2)) {
    case 0:
      FUN_005e4e80(0,DAT_009960f8);
      break;
    case 1:
      FUN_005e4e80(0,DAT_009960fc);
      break;
    case 2:
      FUN_005e4e80(0,DAT_00996100);
      break;
    case 3:
      FUN_005e4e80(0,DAT_00996104);
      break;
    case 4:
      FUN_005e4e80(0,DAT_00996108);
      break;
    case 5:
      FUN_005e4e80(0,DAT_0099610c);
    }
    FUN_005e4e80(1,local_14);
    FUN_00544210(local_18,param_5,0);
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005adcf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005adcf0: 16-byte aligned: YES
  005adcf0  55             PUSH EBP
  005adcf1  8B EC          MOV EBP,ESP
  005adcf3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005adcf0
string literals (1):
  0x00992E8C  at 005add18  'Value'
direct callees (6):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 10):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x2 [dec 2]  x1  via ECX
  +0x6 [dec 6]  x1  via EAX
  +0xA [dec 10]  x1  via ECX
  +0x14 [dec 20]  x1  via ECX
  +0x18 [dec 24]  x1  via EDX
  +0x130 [dec 304]  x1  via EDX
  +0x150 [dec 336]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EDX
```

## Card B0-8-16

GOG function: CGameEffect::Constructor_2  gog 0x00734780  callconv=(blank)  param_size_bytes=4  gog_size=469  gog_retn=4
Candidate Steam address: 0x005e4ac0 (size 469, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005e4920:STR0/CALLEE0/SIG0.971/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007345e0 CGameEffect::Constructor -> 0x005e4920  gate_class=REVIEW
- GOG-order neighbour after:  0x00734960 CGameEffect::Destructor -> 0x005e4ca0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x006059e0 CExoString::operator=_2 -> Steam ?  in candidate callees: n/a
  0x00734a90 CGameEffect::SetNumIntegers -> Steam ?  in candidate callees: n/a
  0x00924260 Global::memset -> Steam ?  in candidate callees: n/a
  0x009258bf Global::`eh_vector_constructor_iterator' -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 17 common): GOG {0x4*,0x8*,0xa*,0xc*,0x10*,0x14*,0x18*,0x1c*,0x20*,0x24*,0x28*,0x2c*,0x30*,0x34*,...(+3)} | Steam {0x4*,0x8*,0xa*,0xc*,0x10*,0x14*,0x18*,0x1c*,0x20*,0x24*,0x28*,0x2c*,0x30*,0x34*,...(+3)}
imm sets (* = in both, 8 common): GOG {0x4*,0x6*,0x8*,0xc*,0x10*,0x38*,0x48*,0x7f000000*} | Steam {0x4*,0x6*,0x8*,0xc*,0x10*,0x38*,0x48*,0x7f000000*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005e4ac0: 16-byte aligned: YES
  005e4ac0  55             PUSH EBP
  005e4ac1  8B EC          MOV EBP,ESP
  005e4ac3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005e4ac0
string literals (0):
direct callees (4):
  0x0091D67E  (unnamed)
  0x005E4DD0  CGameEffect::SetNumIntegers
  0x0091C630  (unnamed)
  0x007338D0  CExoString::operator=_2
struct offsets touched (top 10):
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
  +0xA [dec 10]  x3  via EAX,ECX,EDX
  +0xC [dec 12]  x3  via ECX,EDX
  +0x10 [dec 16]  x3  via EAX,ECX,EDX
  +0x14 [dec 20]  x3  via EAX,ECX
  +0x18 [dec 24]  x3  via EAX,ECX
  +0x1C [dec 28]  x3  via EAX,ECX,EDX
  +0x8 [dec 8]  x1  via ECX
  +0x20 [dec 32]  x1  via EDX
  +0x24 [dec 36]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005e4ac0 FUN_005e4ac0 size=469 ret=4 callers=28 callees=4

undefined4 * __thiscall FUN_005e4ac0(undefined4 *param_1,undefined4 *param_2)

{
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00953e16;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  _eh_vector_constructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable](param_1 + 0x12,8,6,FUN_00733540,FUN_00733780);
  local_8 = 0;
  *param_1 = 0;
  param_1[1 = byte +0x4] = 0;
  param_1[8 = byte +0x20] = 0;
  *(undefined2 *)(param_1 + 0x2 [dec 2] {undefined4*: byte +0x8}) = 0;
  *(undefined2 *)((int)param_1 + 0xa [dec 10]) = 0;
  param_1[3 = byte +0xc] = 0;
  param_1[4 = byte +0x10] = 0;
  param_1[5 = byte +0x14] = 0;
  param_1[6 = byte +0x18] = 0x7f000000;
  param_1[10 = byte +0x28] = 0;
  param_1[0xb = byte +0x2c] = 0;
  param_1[0xd = byte +0x34] = 0;
  param_1[7 = byte +0x1c] = 0xffffffff;
  param_1[9 = byte +0x24] = 0;
  FUN_005e4dd0(param_2[0xc = byte +0x30]);
  _memset [GHIDRA-AUTO-LABEL: unreliable](param_1 + 0xe,0,0x10);
  for (local_14 = 0; local_14 < 6; local_14 = local_14 + 1) {
    FUN_007338d0(&DAT_00986925);
  }
  for (local_14 = 0; local_14 < 4; local_14 = local_14 + 1) {
    param_1[local_14 + 0x1e] = 0x7f000000;
  }
  if (param_2 != (undefined4 *)0x0) {
    *param_1 = *param_2;
    param_1[1 = byte +0x4] = param_2[1 = byte +0x4];
    *(undefined2 *)((int)param_1 + 0xa [dec 10]) = *(undefined2 *)((int)param_2 + 0xa [dec 10]);
    param_1[3 = byte +0xc] = param_2[3 = byte +0xc];
    param_1[4 = byte +0x10] = param_2[4 = byte +0x10];
    param_1[5 = byte +0x14] = param_2[5 = byte +0x14];
    param_1[6 = byte +0x18] = param_2[6 = byte +0x18];
    param_1[7 = byte +0x1c] = param_2[7 = byte +0x1c];
    param_1[0x22 = byte +0x88] = 0;
  }
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x005e4920 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005e4920: 16-byte aligned: YES
  005e4920  55             PUSH EBP
  005e4921  8B EC          MOV EBP,ESP
  005e4923  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005e4920
string literals (0):
direct callees (4):
  0x0091D67E  (unnamed)
  0x005E4DD0  CGameEffect::SetNumIntegers
  0x0091C630  (unnamed)
  0x007338D0  CExoString::operator=_2
struct offsets touched (top 10):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x1  via EDX
  +0xA [dec 10]  x1  via ECX
  +0xC [dec 12]  x1  via EDX
  +0x10 [dec 16]  x1  via EAX
  +0x14 [dec 20]  x1  via ECX
  +0x18 [dec 24]  x1  via EDX
  +0x1C [dec 28]  x1  via EAX
  +0x20 [dec 32]  x1  via ECX
  +0x24 [dec 36]  x1  via EDX
```

## Card B0-8-17

GOG function: CRes::Destructor  gog 0x0061aad0  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x00710c20 (size 113, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00710ca0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061aac0 CSWMGTargettingGunBank::IsTargetting  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0061ab00 CRes::Destructor_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0061ab00 CRes::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
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

### decoy: 0x00710ca0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00710ca0: 16-byte aligned: YES
  00710ca0  55             PUSH EBP
  00710ca1  8B EC          MOV EBP,ESP
  00710ca3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00710ca0
string literals (0):
direct callees (1):
  0x0091E01A  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0xC [dec 12]  x2  via EAX,EDX
```

## Card B0-8-18

GOG function: CSWSCreatureStats::GetTotalSTRBonus  gog 0x006f6e70  callconv=(blank)  param_size_bytes=0  gog_size=70  gog_retn=0
Candidate Steam address: 0x006baa40 (size 70, retn 0)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSCreatureStats::GetTotalDEXBonus
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006ba9f0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006f6c50 CSWSCreatureStats::GetFeat -> 0x006ba7d0  gate_class=AUTO
- GOG-order neighbour after:  0x006f6ec0 CSWSCreatureStats::GetTotalDEXBonus -> 0x006baa40  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0069aaf0 CSWSCreature::GetTotalEffectBonus -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x24*} | Steam {0x24*}
imm sets (* = in both, 3 common): GOG {0x4*,0x10*,0xff*} | Steam {0x4*,0x10*,0xff*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x006baa40: 16-byte aligned: YES
  006baa40  55             PUSH EBP
  006baa41  8B EC          MOV EBP,ESP
  006baa43  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006baa40
string literals (0):
direct callees (1):
  0x00582010  CSWSCreature::GetTotalEffectBonus
struct offsets touched (top 1):
  +0x24 [dec 36]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 006baa40 FUN_006baa40 size=70 ret=0 callers=4 callees=1

undefined4 __fastcall FUN_006baa40(int param_1)

{
  undefined4 uVar1;
  undefined4 local_c[2 = byte +0x8];
  
  local_c[0] = 0;
  uVar1 = FUN_00582010(4,0,local_c,0,0,0,0,0xff,1,0,0,param_1,*(undefined4 *)(param_1 + 0x24));
  return uVar1;
}
```

### decoy: 0x006ba9f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006ba9f0: 16-byte aligned: YES
  006ba9f0  55             PUSH EBP
  006ba9f1  8B EC          MOV EBP,ESP
  006ba9f3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006ba9f0
string literals (0):
direct callees (1):
  0x00582010  CSWSCreature::GetTotalEffectBonus
struct offsets touched (top 1):
  +0x24 [dec 36]  x1  via EAX
```

## Card B0-8-19

GOG function: CSWSEffectListHandler::OnEffectApplied  gog 0x00737020  callconv=(blank)  param_size_bytes=12  gog_size=96  gog_retn=12
Candidate Steam address: 0x00594740 (size 96, retn 12)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005947a0:STR0/CALLEE0/SIG0.583/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00736310 CSWSEffectListHandler::InitializeEffects -> 0x00593a30  gate_class=REVIEW
- GOG-order neighbour after:  0x00737080 CSWSEffectListHandler::OnEffectRemoved -> 0x005947a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
imm sets (* = in both, 2 common): GOG {0xc*,0x76*} | Steam {0xc*,0x76*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x00594740: 16-byte aligned: YES
  00594740  55             PUSH EBP
  00594741  8B EC          MOV EBP,ESP
  00594743  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00594740
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x2  via EAX
  +0x8 [dec 8]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00594740 FUN_00594740 size=96 ret=12 callers=0 callees=0

undefined4 __thiscall FUN_00594740(int param_1,undefined4 param_2,int param_3,undefined4 param_4)

{
  undefined4 uVar1;
  uint uVar2;
  
  uVar2 = (uint)*(ushort *)(param_3 + 8);
  if (((uVar2 == 0) || (0x75 < uVar2)) || (*(int *)(*(int *)(param_1 + 4) + uVar2 * 4) == 0)) {
    uVar1 = 1;
  }
  else {
    uVar1 = (**(code **)(*(int *)(param_1 + 4) + uVar2 * 4))(param_2,param_3,param_4);
  }
  return uVar1;
}
```

### decoy: 0x005947a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005947a0: 16-byte aligned: YES
  005947a0  55             PUSH EBP
  005947a1  8B EC          MOV EBP,ESP
  005947a3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005947a0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x8 [dec 8]  x4  via EAX,EDX
```

## Card B0-8-20

GOG function: CGameEffect::GetExpiryTime  gog 0x00734c60  callconv=(blank)  param_size_bytes=8  gog_size=35  gog_retn=8
Candidate Steam address: 0x005e4fa0 (size 35, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005e4f80:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00734c40 CGameEffect::SetExpiryTime -> 0x005e4f80  gate_class=REVIEW
- GOG-order neighbour after:  0x00734c90 CGameEffect::CopyEffect -> 0x005e4fd0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x10*,0x14*} | Steam {0x10*,0x14*}
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
prologue for 0x005e4fa0: 16-byte aligned: YES
  005e4fa0  55             PUSH EBP
  005e4fa1  8B EC          MOV EBP,ESP
  005e4fa3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005e4fa0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x10 [dec 16]  x1  via ECX
  +0x14 [dec 20]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005e4fa0 FUN_005e4fa0 size=35 ret=8 callers=3 callees=0

void __thiscall FUN_005e4fa0(int param_1,undefined4 *param_2,undefined4 *param_3)

{
  *param_2 = *(undefined4 *)(param_1 + 0x10);
  *param_3 = *(undefined4 *)(param_1 + 0x14);
  return;
}
```

### decoy: 0x005e4f80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005e4f80: 16-byte aligned: YES
  005e4f80  55             PUSH EBP
  005e4f81  8B EC          MOV EBP,ESP
  005e4f83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005e4f80
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x10 [dec 16]  x1  via EAX
  +0x14 [dec 20]  x1  via EDX
```

## Card B0-8-21

GOG function: CSWSEffectListHandler::OnApplyMovementSpeedIncrease  gog 0x00741260  callconv=(blank)  param_size_bytes=8  gog_size=145  gog_retn=12
Candidate Steam address: 0x0059de30 (size 2536, retn 12)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSEffectListHandler::OnApplySetStateInternal
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.06 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0059e860:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00741150 CSWSEffectListHandler::OnRemoveSetStateInternal -> 0x0059e860  gate_class=REVIEW
- GOG-order neighbour after:  0x00741300 CSWSEffectListHandler::OnRemoveMovementSpeedIncrease -> 0x0059ea10  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0068f140 CSWSCreature::GetMovementRateFactor -> Steam ?  in candidate callees: n/a
  0x0068f1d0 CSWSCreature::SetMovementRateFactor -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x30*} | Steam {0x4,0x8,0x18,0x30*,0xec,0x122,0x145,0x154,0x390,0x394,0xabc,0x10dc}
imm sets (* = in both, 1 common): GOG {0xc*,0x14,0x64} | Steam {0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc*,0x11,0x17,0x1b,...(+8)}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x0059de30: 16-byte aligned: YES
  0059de30  55             PUSH EBP
  0059de31  8B EC          MOV EBP,ESP
  0059de33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0059de30
string literals (0):
direct callees (13):
  0x005E4E60  CGameEffect::GetInteger
  0x00541080  CSWSObject::ClearAllActions
  0x00590430  CSWSCombatRound::RemoveAllActions
  0x00919723  operator new
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E80  CGameEffect::SetInteger
  0x005E4FA0  CGameEffect::GetExpiryTime
  0x005E4F80  CGameEffect::SetExpiryTime
  0x00544210  CSWSObject::ApplyEffect
  0x0057A990  CSWSCreature::RecomputeAmbientAnimationState
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x005908C0  CSWSCombatRound::GetAttack
  0x006E6E70  (unnamed)
struct offsets touched (top 10):
  +0x8 [dec 8]  x14  via EAX,ECX,EDX
  +0x10DC [dec 4316]  x3  via EAX,ECX,EDX
  +0xEC [dec 236]  x2  via EAX
  +0x122 [dec 290]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x18 [dec 24]  x1  via EAX
  +0x30 [dec 48]  x1  via EDX
  +0x145 [dec 325]  x1  via ECX
  +0x154 [dec 340]  x1  via EAX
  +0x390 [dec 912]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0059de30 FUN_0059de30 size=2536 ret=12 callers=0 callees=13

undefined4 FUN_0059de30(int *param_1,int param_2,undefined4 param_3)

{
  int iVar1;
  bool bVar2;
  int local_12c;
  int local_128;
  int local_124;
  int local_120;
  int local_11c;
  int local_118;
  int local_114;
  int local_110;
  int local_10c;
  int local_108;
  int local_104;
  int local_100;
  int local_fc;
  undefined4 local_38;
  int local_34;
  undefined4 local_30;
  int local_2c;
  int local_28;
  int local_24;
  int local_20;
  undefined4 local_1c;
  uint local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095232d;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_20 = (**(code **)(*param_1 + 0x30))(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_18 = 0;
  if ((local_20 != 0) && (param_2 != 0)) {
    local_14 = FUN_005e4e60(0);
    local_28 = 0;
    *(undefined4 *)(local_20 + 0x390) = 0;
    *(undefined4 *)(local_20 + 0x394) = 1;
    local_18 = 0;
    bVar2 = local_14 != 9;
    if (bVar2) {
      *(undefined4 *)(local_20 + 0xec) = 1;
      FUN_00541080(1);
      FUN_00590430();
      *(undefined4 *)(local_20 + 0xec) = 0;
    }
    local_18 = (uint)bVar2;
    switch(local_14) {
    case 2:
      iVar1 = FUN_00919723(0x8c);
      local_8 = 4;
      if (iVar1 == 0) {
        local_10c = 0;
      }
      else {
        local_10c = FUN_005e4ac0(param_2);
      }
      local_8 = 0xffffffff;
      *(undefined2 *)(local_10c + 8) = 0x1b;
      FUN_005e4e80(0,2);
      FUN_005e4e80(1,0);
      FUN_005e4e80(3,*(undefined1 *)(DAT_00a1b4d0 + 0x122));
      FUN_00544210(local_10c,param_3,0);
      break;
    case 3:
      iVar1 = FUN_00919723(0x8c);
      local_8 = 5;
      if (iVar1 == 0) {
        local_110 = 0;
      }
      else {
        local_110 = FUN_005e4ac0(param_2);
      }
      local_8 = 0xffffffff;
/* ... 187 more lines truncated by --max-lines */
```

### decoy: 0x0059e860 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059e860: 16-byte aligned: YES
  0059e860  55             PUSH EBP
  0059e861  8B EC          MOV EBP,ESP
  0059e863  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059e860
string literals (0):
direct callees (5):
  0x005E4E60  CGameEffect::GetInteger
  0x006FD8D0  CVirtualMachine::RunScript
  0x0057A990  CSWSCreature::RecomputeAmbientAnimationState
  0x005D1510  CSWSCreature::ClearVisibleList
  0x005D0F30  CSWSObject::UpdatePerception
struct offsets touched (top 4):
  +0x4 [dec 4]  x1  via ECX
  +0x30 [dec 48]  x1  via EDX
  +0x10E8 [dec 4328]  x1  via ECX
  +0x59E958 [dec 5892440]  x1  via EDX
```

## Card B0-8-22

GOG function: CSWSArea::NoNonWalkPolys  gog 0x007221c0  callconv=(blank)  param_size_bytes=32  gog_size=74  gog_retn=32
Candidate Steam address: 0x0052d0e0 (size 74, retn 32)
Candidate sources: branch+mapper:vote+name_map:high:batch5_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (UpdateAreaTransitionDisplay 0x87c9f0)
- best decoy (neighbour/other candidate) scores: 0x0052ccd0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00721db0 CSWSArea::LoadPathPoints -> 0x0052ccd0  gate_class=AUTO
- GOG-order neighbour after:  0x00722210 CSWSArea::NoNonWalkPolysDetailed -> 0x0052d130  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00722210 CSWSArea::NoNonWalkPolysDetailed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x8*,0x20*} | Steam {0x8*,0x20*}

### abi
```
GOG param bytes: 32; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(32/4)*4 = 32 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=32 ; asm RET = 32
Stack Slot Match: TRUE (GOG 32 bytes -> x86 slots 32; ASM RET 32)
```

### prologue
```
prologue for 0x0052d0e0: 16-byte aligned: YES
  0052d0e0  55             PUSH EBP
  0052d0e1  8B EC          MOV EBP,ESP
  0052d0e3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0052d0e0
string literals (0):
direct callees (1):
  0x0052D130  CSWSArea::NoNonWalkPolysDetailed
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0052d0e0 FUN_0052d0e0 size=74 ret=32 callers=3 callees=1
THUNK -> 0x0052D130 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0052d0e0(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
                 undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8)

{
  undefined1 local_8 [4];
  
  FUN_0052d130(param_1,param_2,param_3,param_4,param_5,param_6,local_8,0,0,0,param_7,param_8);
  return;
}
```

### decoy: 0x0052ccd0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0052ccd0: 16-byte aligned: YES
  0052ccd0  55             PUSH EBP
  0052ccd1  8B EC          MOV EBP,ESP
  0052ccd3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0052ccd0
string literals (5):
  0x00993944  at 0052cdda  'Path_Points'
  0x00993938  at 0052cf44  'Conections'
  0x00993928  at 0052cf77  'First_Conection'
  0x00993918  at 0052cfa9  'Path_Conections'
  0x0099390C  at 0052d05b  'Destination'
direct callees (11):
  0x00711ED0  CExoResMan::Exists
  0x00919723  operator new
  0x00716FD0  CResGFF::Constructor_2
  0x007189D0  CResGFF::GetTopLevelStruct
  0x00718760  CResGFF::GetList
  0x00718810  CResGFF::GetListCount
  0x00718890  CResGFF::GetListElement
  0x00718690  CResGFF::GetElementType
  0x00718D00  CResGFF::ReadFieldFLOAT
  0x00718C00  CResGFF::ReadFieldDWORD
  0x009196EE  __security_check_cookie
struct offsets touched (top 6):
  +0x260 [dec 608]  x5  via EAX,ECX
  +0x25C [dec 604]  x4  via EAX,EDX
  +0x264 [dec 612]  x3  via EAX,ECX,EDX
  +0x268 [dec 616]  x2  via EAX,EDX
  +0x94 [dec 148]  x1  via EAX
  +0x10C [dec 268]  x1  via EAX
```

## Card B0-8-23

GOG function: CSWSEffectListHandler::OnRemoveFury  gog 0x00737990  callconv=(blank)  param_size_bytes=8  gog_size=225  gog_retn=8
Candidate Steam address: 0x005950b0 (size 225, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00594f50:STR0/CALLEE0/SIG0.336/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00737830 CSWSEffectListHandler::OnApplyFury -> 0x00594f50  gate_class=REVIEW
- GOG-order neighbour after:  0x00737a80 CSWCSoundObject::LoadModel  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x4*,0x8*,0x30*,0x120*,0x124*,0x1198*} | Steam {0x4*,0x8*,0x30*,0x120*,0x124*,0x1198*}
imm sets (* = in both, 4 common): GOG {0x8*,0x28*,0x6f*,0x148*} | Steam {0x8*,0x28*,0x6f*,0x148*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x005950b0: 16-byte aligned: YES
  005950b0  55             PUSH EBP
  005950b1  8B EC          MOV EBP,ESP
  005950b3  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005950b0
string literals (0):
direct callees (0):
struct offsets touched (top 6):
  +0x30 [dec 48]  x2  via EDX
  +0x1198 [dec 4504]  x2  via EDX
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
  +0x120 [dec 288]  x1  via ECX
  +0x124 [dec 292]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005950b0 FUN_005950b0 size=225 ret=8 callers=0 callees=0

undefined4 __thiscall FUN_005950b0(undefined4 param_1,int *param_2,int param_3)

{
  int iVar1;
  bool bVar2;
  int iVar3;
  undefined4 local_18;
  
  if (param_2 != (int *)0x0) {
    iVar3 = (**(code **)(*param_2 + 0x30))(param_1);
    if (iVar3 != 0) {
      iVar3 = (**(code **)(*param_2 + 0x30))();
      bVar2 = false;
      for (local_18 = 0; local_18 < *(int *)(iVar3 + 0x14c); local_18 = local_18 + 1) {
        iVar1 = *(int *)(*(int *)(iVar3 + 0x148) + local_18 * 4);
        if ((*(short *)(iVar1 + 8) == 0x6f) && (iVar1 != param_3)) {
          bVar2 = true;
        }
      }
      if (!bVar2) {
        *(undefined1 *)(*(int *)(iVar3 + 0x1198) + 0x120) = 0xff;
        *(undefined4 *)(*(int *)(iVar3 + 0x1198) + 0x124) = 0;
      }
    }
  }
  return 1;
}
```

### decoy: 0x00594f50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00594f50: 16-byte aligned: YES
  00594f50  55             PUSH EBP
  00594f51  8B EC          MOV EBP,ESP
  00594f53  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00594f50
string literals (0):
direct callees (0):
struct offsets touched (top 6):
  +0x1198 [dec 4504]  x4  via ECX
  +0x124 [dec 292]  x3  via EAX
  +0x30 [dec 48]  x2  via EDX
  +0x1C [dec 28]  x1  via EDX
  +0x120 [dec 288]  x1  via EAX
  +0x595038 [dec 5853240]  x1  via EDX
```

## Card B0-8-24

GOG function: CGameEffect::SetNumIntegers  gog 0x00734a90  callconv=(blank)  param_size_bytes=4  gog_size=134  gog_retn=4
Candidate Steam address: 0x005e4dd0 (size 134, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005e4e60:STR0/CALLEE0/SIG0.417/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00734960 CGameEffect::Destructor -> 0x005e4ca0  gate_class=REVIEW
- GOG-order neighbour after:  0x00734b20 CGameEffect::GetInteger -> 0x005e4e60  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
  0x00924260 Global::memset -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x30*,0x34*} | Steam {0x30*,0x34*}
imm sets (* = in both, 3 common): GOG {0x2*,0x4*,0xc*} | Steam {0x2*,0x4*,0xc*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005e4dd0: 16-byte aligned: YES
  005e4dd0  55             PUSH EBP
  005e4dd1  8B EC          MOV EBP,ESP
  005e4dd3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005e4dd0
string literals (0):
direct callees (3):
  0x009196FD  operator delete
  0x00919723  operator new
  0x0091C630  (unnamed)
struct offsets touched (top 2):
  +0x34 [dec 52]  x5  via EAX,ECX,EDX
  +0x30 [dec 48]  x3  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005e4dd0 FUN_005e4dd0 size=134 ret=4 callers=14 callees=3

void __thiscall FUN_005e4dd0(int param_1,undefined4 param_2)

{
  longlong lVar1;
  undefined4 uVar2;
  
  *(undefined4 *)(param_1 + 0x30) = param_2;
  if (*(int *)(param_1 + 0x34) != 0) {
    FUN_009196fd(*(undefined4 *)(param_1 + 0x34));
    *(undefined4 *)(param_1 + 0x34) = 0;
  }
  lVar1 = (ulonglong)*(uint *)(param_1 + 0x30) * 4;
  uVar2 = FUN_00919723(-(uint)((int)((ulonglong)lVar1 >> 0x20) != 0) | (uint)lVar1);
  *(undefined4 *)(param_1 + 0x34) = uVar2;
  _memset [GHIDRA-AUTO-LABEL: unreliable](*(void **)(param_1 + 0x34),0,*(int *)(param_1 + 0x30) << 2);
  return;
}
```

### decoy: 0x005e4e60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005e4e60: 16-byte aligned: YES
  005e4e60  55             PUSH EBP
  005e4e61  8B EC          MOV EBP,ESP
  005e4e63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005e4e60
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x34 [dec 52]  x1  via EAX
```

## Card B0-8-25

GOG function: CVirtualMachineInternal::RunScriptFile  gog 0x00645af0  callconv=(blank)  param_size_bytes=4  gog_size=345  gog_retn=4
Candidate Steam address: 0x007029f0 (size 353, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CVirtualMachineInternal::SetUpScriptSituation
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.27 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00702b60:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00645830 CVirtualMachineInternal::RunScriptSituation -> 0x00702590  gate_class=REVIEW
- GOG-order neighbour after:  0x00645c50 CVirtualMachineInternal::GetRunScriptReturnValue -> 0x007029b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  0x00642680 CVirtualMachineStack::GetStackPointer -> Steam ?  in candidate callees: n/a
  0x00642690 CVirtualMachineStack::SetStackPointer -> Steam ?  in candidate callees: n/a
  0x00642c30 CVirtualMachineInternal::ExecuteCode -> Steam ?  in candidate callees: n/a
  0x00645220 CVirtualMachineInternal::PushInstructionPtr -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x10*,0x1c,0x20*,0x28*,0x3c*,0x40*,0x44*,0x1ac,0x1c4,0x3d4} | Steam {0x4,0x8,0xc,0x10*,0x14,0x20*,0x24,0x28*,0x2c,0x30,0x34,0x38,0x3c*,0x40*,...(+3)}
imm sets (* = in both, 2 common): GOG {0x4*,0x1c,0x28*,0x1ac} | Steam {0x4*,0x8,0xc,0x18,0x28*,0xffffffa2}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007029f0: 16-byte aligned: YES
  007029f0  55             PUSH EBP
  007029f1  8B EC          MOV EBP,ESP
  007029f3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007029f0
string literals (0):
direct callees (2):
  0x00701FD0  CVirtualMachineInternal::ReadScriptFile
  0x007337C0  CExoString::operator=
struct offsets touched (top 8):
  +0x28 [dec 40]  x14  via EAX,ECX,EDX
  +0x10 [dec 16]  x4  via EAX,ECX,EDX
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via EAX
  +0xC [dec 12]  x1  via ECX
  +0x14 [dec 20]  x1  via EDX
  +0x20 [dec 32]  x1  via EAX
  +0x24 [dec 36]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007029f0 FUN_007029f0 size=353 ret=4 callers=1 callees=2

int __thiscall FUN_007029f0(int param_1,undefined4 *param_2)

{
  int iVar1;
  undefined4 local_8;
  
  local_8 = param_2[8 = byte +0x20];
  if (param_2[4 = byte +0x10] == 0) {
    iVar1 = FUN_00701fd0(param_2 + 0x6 [dec 6] {undefined4*: byte +0x18});
    if (iVar1 != 0) {
      return iVar1;
    }
    local_8 = 0;
  }
  else {
    *(int *)(param_1 + 0x28) = *(int *)(param_1 + 0x28) + 1;
    if (7 < *(int *)(param_1 + 0x28)) {
      *(int *)(param_1 + 0x28) = *(int *)(param_1 + 0x28) + -1;
      return -0x5e;
    }
  }
  *(undefined4 *)(param_1 + 0x4c + *(int *)(param_1 + 0x28) * 0x28) = local_8;
  FUN_007337c0(param_2 + 0x6 [dec 6] {undefined4*: byte +0x18});
  *(undefined4 *)(param_1 + 0x2c + *(int *)(param_1 + 0x28) * 0x28) = *param_2;
  *(undefined4 *)(param_1 + 0x30 + *(int *)(param_1 + 0x28) * 0x28) = param_2[1 = byte +0x4];
  *(undefined4 *)(param_1 + 0x34 + *(int *)(param_1 + 0x28) * 0x28) = param_2[2 = byte +0x8];
  *(undefined4 *)(param_1 + 0x38 + *(int *)(param_1 + 0x28) * 0x28) = param_2[3 = byte +0xc];
  *(undefined4 *)(param_1 + 0x50 + *(int *)(param_1 + 0x28) * 0x28) = param_2[9 = byte +0x24];
  if (param_2[4 = byte +0x10] != 0) {
    *(undefined4 *)(param_1 + 0x3c + *(int *)(param_1 + 0x28) * 0x28) = param_2[4 = byte +0x10];
    *(undefined4 *)(param_1 + 0x40 + *(int *)(param_1 + 0x28) * 0x28) = param_2[5 = byte +0x14];
  }
  *param_2 = 0;
  param_2[4 = byte +0x10] = 0;
  return 0;
}
```

### decoy: 0x00702b60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00702b60: 16-byte aligned: YES
  00702b60  55             PUSH EBP
  00702b61  8B EC          MOV EBP,ESP
  00702b63  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00702b60
string literals (0):
direct callees (2):
  0x006FF440  CVirtualMachineStack::GetStackPointer
  0x006FF450  CVirtualMachineStack::SetStackPointer
struct offsets touched (top 2):
  +0x1B8 [dec 440]  x1  via ECX
  +0x1BC [dec 444]  x1  via EDX
```

## Card B0-8-26

GOG function: CSWSEffectListHandler::InitializeEffects  gog 0x00736310  callconv=(blank)  param_size_bytes=0  gog_size=3329  gog_retn=0
Candidate Steam address: 0x00593a30 (size 3329, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00594740:STR0/CALLEE0/SIG0.109/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007362f0 CSWSEffectListHandler::OnRemoveForceSight -> 0x00593a10  gate_class=REVIEW
- GOG-order neighbour after:  0x00737020 CSWSEffectListHandler::OnEffectApplied -> 0x00594740  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 116 common): GOG {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x1c*,0x20*,0x24*,0x28*,0x2c*,0x30*,0x34*,0x38*,...(+102)} | Steam {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x1c*,0x20*,0x24*,0x28*,0x2c*,0x30*,0x34*,0x38*,...(+102)}
imm sets (* = in both, 4 common): GOG {0x4*,0x10*,0x76*,0x1d8*} | Steam {0x4*,0x10*,0x76*,0x1d8*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00593a30: 16-byte aligned: YES
  00593a30  55             PUSH EBP
  00593a31  8B EC          MOV EBP,ESP
  00593a33  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00593a30
string literals (0):
direct callees (1):
  0x00919723  operator new
struct offsets touched (top 10):
  +0x4 [dec 4]  x120  via EAX,ECX,EDX
  +0x8 [dec 8]  x98  via EAX,ECX,EDX
  +0xC [dec 12]  x2  via ECX,EDX
  +0x14 [dec 20]  x2  via EAX,EDX
  +0x18 [dec 24]  x2  via EAX,ECX
  +0x20 [dec 32]  x2  via EAX,ECX
  +0x24 [dec 36]  x2  via ECX,EDX
  +0x28 [dec 40]  x2  via ECX,EDX
  +0x2C [dec 44]  x2  via EAX,EDX
  +0x34 [dec 52]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00593a30 FUN_00593a30 size=3329 ret=0 callers=1 callees=1

void __fastcall FUN_00593a30(int param_1)

{
  undefined4 uVar1;
  int local_8;
  
  uVar1 = FUN_00919723(0x1d8);
  *(undefined4 *)(param_1 + 4) = uVar1;
  uVar1 = FUN_00919723(0x1d8);
  *(undefined4 *)(param_1 + 8) = uVar1;
  for (local_8 = 0; local_8 < 0x76; local_8 = local_8 + 1) {
    *(undefined4 *)(*(int *)(param_1 + 4) + local_8 * 4) = 0;
    *(undefined4 *)(*(int *)(param_1 + 8) + local_8 * 4) = 0;
  }
  *(code **)(*(int *)(param_1 + 4) + 0x98) = FUN_005959e0;
  *(code **)(*(int *)(param_1 + 4) + 0x9c) = FUN_00597750;
  *(code **)(*(int *)(param_1 + 4) + 0x4c) = FUN_00597d90;
  *(code **)(*(int *)(param_1 + 4) + 0x10) = FUN_00599050;
  *(code **)(*(int *)(param_1 + 4) + 0x90) = FUN_00599260;
  *(code **)(*(int *)(param_1 + 8) + 0x90) = FUN_00599330;
  *(code **)(*(int *)(param_1 + 4) + 0x94) = FUN_00599380;
  *(code **)(*(int *)(param_1 + 8) + 0x94) = FUN_005994b0;
  *(code **)(*(int *)(param_1 + 4) + 0x18) = FUN_00599500;
  *(code **)(*(int *)(param_1 + 8) + 0x18) = FUN_00599d90;
  *(code **)(*(int *)(param_1 + 4) + 8) = FUN_005a65c0;
  *(code **)(*(int *)(param_1 + 4) + 0x68) = FUN_00599de0;
  *(code **)(*(int *)(param_1 + 8) + 0x68) = FUN_0059aa40;
  *(code **)(*(int *)(param_1 + 4) + 0x6c) = FUN_00599e40;
  *(code **)(*(int *)(param_1 + 8) + 0x6c) = FUN_0059aa40;
  *(code **)(*(int *)(param_1 + 4) + 0xc0) = FUN_00599f00;
  *(code **)(*(int *)(param_1 + 8) + 0xc0) = FUN_0059a200;
  *(code **)(*(int *)(param_1 + 4) + 0xc4) = FUN_0059a4b0;
  *(code **)(*(int *)(param_1 + 8) + 0xc4) = FUN_0059a730;
  *(code **)(*(int *)(param_1 + 4) + 0x28) = FUN_0059a9e0;
  *(code **)(*(int *)(param_1 + 8) + 0x28) = FUN_0059aa40;
  *(code **)(*(int *)(param_1 + 4) + 0x2c) = FUN_0059aa90;
  *(code **)(*(int *)(param_1 + 8) + 0x2c) = FUN_0059aa40;
  *(code **)(*(int *)(param_1 + 4) + 0x34) = FUN_0059ab70;
  *(code **)(*(int *)(param_1 + 8) + 0x34) = FUN_0059aa40;
  *(code **)(*(int *)(param_1 + 4) + 0x38) = FUN_0059abb0;
  *(code **)(*(int *)(param_1 + 8) + 0x38) = FUN_0059aa40;
  *(code **)(*(int *)(param_1 + 4) + 0x3c) = FUN_0059ac50;
  *(code **)(*(int *)(param_1 + 8) + 0x3c) = FUN_0059acc0;
  *(code **)(*(int *)(param_1 + 4) + 0x40) = FUN_0059b130;
  *(code **)(*(int *)(param_1 + 8) + 0x40) = FUN_0059b1a0;
  *(code **)(*(int *)(param_1 + 4) + 0x44) = FUN_0059b2e0;
  *(code **)(*(int *)(param_1 + 8) + 0x44) = FUN_0059b3e0;
  *(code **)(*(int *)(param_1 + 4) + 0x48) = FUN_0059b520;
  *(code **)(*(int *)(param_1 + 8) + 0x48) = FUN_0059b870;
  *(code **)(*(int *)(param_1 + 4) + 0x50) = FUN_0059b8d0;
  *(code **)(*(int *)(param_1 + 8) + 0x50) = FUN_0059be80;
  *(code **)(*(int *)(param_1 + 4) + 0x15c) = FUN_0059bf80;
  *(code **)(*(int *)(param_1 + 8) + 0x15c) = FUN_0059f6f0;
  *(code **)(*(int *)(param_1 + 4) + 0x5c) = FUN_0059c3b0;
  *(code **)(*(int *)(param_1 + 8) + 0x5c) = FUN_0059c420;
  *(code **)(*(int *)(param_1 + 4) + 0x54) = FUN_0059c550;
  *(code **)(*(int *)(param_1 + 4) + 0x60) = FUN_0059c7f0;
  *(code **)(*(int *)(param_1 + 8) + 0x60) = FUN_0059c890;
  *(code **)(*(int *)(param_1 + 4) + 0x20) = FUN_0059cba0;
  *(code **)(*(int *)(param_1 + 8) + 0x20) = FUN_0059db50;
  *(code **)(*(int *)(param_1 + 4) + 0x148) = FUN_005a65d0;
  *(code **)(*(int *)(param_1 + 8) + 0x148) = FUN_005a6c60;
  *(code **)(*(int *)(param_1 + 4) + 0xf4) = FUN_005a65c0;
  *(code **)(*(int *)(param_1 + 8) + 0xf4) = FUN_0059f6f0;
  *(code **)(*(int *)(param_1 + 4) + 0x1c) = FUN_0059fcb0;
  *(code **)(*(int *)(param_1 + 4) + 0x14) = FUN_005a06f0;
  *(code **)(*(int *)(param_1 + 8) + 0x14) = FUN_005a0c00;
  *(code **)(*(int *)(param_1 + 4) + 0x8c) = FUN_0059fd20;
  *(code **)(*(int *)(param_1 + 8) + 0x8c) = FUN_005a0640;
  *(code **)(*(int *)(param_1 + 4) + 0x30) = FUN_005a65c0;
  *(code **)(*(int *)(param_1 + 4) + 0x70) = FUN_0059e970;
  *(code **)(*(int *)(param_1 + 8) + 0x70) = FUN_0059ea10;
  *(code **)(*(int *)(param_1 + 4) + 0x74) = FUN_0059eb80;
  *(code **)(*(int *)(param_1 + 8) + 0x74) = FUN_0059ea10;
  *(code **)(*(int *)(param_1 + 4) + 0x78) = FUN_0059ec70;
  *(code **)(*(int *)(param_1 + 8) + 0x78) = FUN_0059f120;
  *(code **)(*(int *)(param_1 + 4) + 0x7c) = FUN_0059f150;
  *(code **)(*(int *)(param_1 + 8) + 0x7c) = FUN_0059f4f0;
  *(code **)(*(int *)(param_1 + 4) + 0x80) = FUN_0059f5a0;
/* ... 148 more lines truncated by --max-lines */
```

### decoy: 0x00594740 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00594740: 16-byte aligned: YES
  00594740  55             PUSH EBP
  00594741  8B EC          MOV EBP,ESP
  00594743  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00594740
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x2  via EAX
  +0x8 [dec 8]  x1  via EAX
```

## Card B0-8-27

GOG function: CGameEffect::GetObjectID  gog 0x00734ba0  callconv=(blank)  param_size_bytes=4  gog_size=23  gog_retn=4
Candidate Steam address: 0x005e4ee0 (size 23, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005e4f00:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00734b80 CGameEffect::SetFloat -> 0x005e4ec0  gate_class=REVIEW
- GOG-order neighbour after:  0x00734bc0 CGameEffect::SetObjectID -> 0x005e4f00  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x78*} | Steam {0x78*}
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
prologue for 0x005e4ee0: 16-byte aligned: YES
  005e4ee0  55             PUSH EBP
  005e4ee1  8B EC          MOV EBP,ESP
  005e4ee3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005e4ee0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005e4ee0 FUN_005e4ee0 size=23 ret=4 callers=13 callees=0

undefined4 __thiscall FUN_005e4ee0(int param_1,int param_2)

{
  return *(undefined4 *)(param_1 + 0x78 + param_2 * 4);
}
```

### decoy: 0x005e4f00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005e4f00: 16-byte aligned: YES
  005e4f00  55             PUSH EBP
  005e4f01  8B EC          MOV EBP,ESP
  005e4f03  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005e4f00
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

## Card B0-8-28

GOG function: CSWSEffectListHandler::OnRemoveSummonParty  gog 0x0073c680  callconv=(blank)  param_size_bytes=0  gog_size=67  gog_retn=8
Candidate Steam address: 0x00599d90 (size 67, retn 8)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSEffectListHandler::OnApplySummonParty
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00599de0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0073c300 CSWSEffectListHandler::OnApplySummonParty -> 0x00599a10  gate_class=AUTO
- GOG-order neighbour after:  0x0073c6d0 CSWSEffectListHandler::OnApplySavingThrowIncrease -> 0x00599de0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x006e0170 CSWSCreature::UnsummonMyself -> Steam ?  in candidate callees: n/a
  0x00734ba0 CGameEffect::GetObjectID -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 2 common): GOG {0x8*,0xc*} | Steam {0x8*,0xc*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 8)
```

### prologue
```
prologue for 0x00599d90: 16-byte aligned: YES
  00599d90  55             PUSH EBP
  00599d91  8B EC          MOV EBP,ESP
  00599d93  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00599d90
string literals (0):
direct callees (3):
  0x005E4EE0  CGameEffect::GetObjectID
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x005D5550  CSWSCreature::UnsummonMyself
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00599d90 FUN_00599d90 size=67 ret=8 callers=0 callees=3

undefined4 FUN_00599d90(void)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 extraout_var;
  undefined4 uVar3;
  undefined4 uVar4;
  
  uVar4 = *(undefined4 *)(DAT_00a1b4a4 + 8);
  uVar1 = FUN_005e4ee0(1);
  uVar3 = extraout_var;
  iVar2 = FUN_0051c100(uVar1);
  if (iVar2 != 0) {
    FUN_005d5550(uVar3,uVar4);
  }
  return 1;
}
```

### decoy: 0x00599de0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00599de0: 16-byte aligned: YES
  00599de0  55             PUSH EBP
  00599de1  8B EC          MOV EBP,ESP
  00599de3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00599de0
string literals (0):
direct callees (1):
  0x005E4E60  CGameEffect::GetInteger
struct offsets touched (top 2):
  +0x30 [dec 48]  x2  via EDX
  +0x384 [dec 900]  x1  via ECX
```

## Card B0-8-29

GOG function: CSWSArea::Destructor_3  gog 0x00720f20  callconv=(blank)  param_size_bytes=0  gog_size=239  gog_retn=32
Candidate Steam address: 0x0052bb60 (size 767, retn 32)
Candidate sources: near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSArea::ClearLineOfSight
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.63 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.63
- best decoy (neighbour/other candidate) scores: 0x0052be60:STR0/CALLEE0/SIG0.2/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00720dc0 CSWSArea::Constructor_2 -> 0x0052bb60  gate_class=REVIEW
- GOG-order neighbour after:  0x00721010 CSWSArea::ApplyEffect -> 0x0052bf30  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x0060b670 CExoArrayList_T_::Destructor -> Steam ?  in candidate callees: n/a
  0x00734410 CSWSRoom::ClearLineOfSight -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0x254*} | Steam {0x4*,0x8*,0x10,0x250,0x254*}
imm sets (* = in both, 4 common): GOG {0xc*,0x20*,0x41*,0x4c*} | Steam {0xc*,0x20*,0x41*,0x4c*,0xc0,0x7f000000}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=32 ; asm RET = 32
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 32)
```

### prologue
```
prologue for 0x0052bb60: 16-byte aligned: YES
  0052bb60  55             PUSH EBP
  0052bb61  8B EC          MOV EBP,ESP
  0052bb63  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0052bb60
string literals (0):
direct callees (8):
  0x0084A640  (unnamed)
  0x0051C0A0  CServerExoApp::GetGameObject
  0x0083EA60  CExoArrayList<uint32>::Add
  0x0052BE60  CSWSArea::Constructor_2
  0x0052D130  CSWSArea::NoNonWalkPolysDetailed
  0x005FF950  (unnamed)
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0064DDB0  Free(ptr*) delete-and-zero helper
struct offsets touched (top 5):
  +0x8 [dec 8]  x14  via EAX,ECX,EDX
  +0x4 [dec 4]  x6  via EAX,ECX,EDX
  +0x10 [dec 16]  x2  via EAX,EDX
  +0x250 [dec 592]  x1  via ECX
  +0x254 [dec 596]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0052bb60 FUN_0052bb60 size=767 ret=32 callers=20 callees=8

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall
FUN_0052bb60(int param_1,undefined4 *param_2,undefined4 *param_3,undefined4 *param_4,
            undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8,int param_9)

{
  float fVar1;
  undefined4 uVar2;
  uint uVar3;
  int iVar4;
  float local_9c;
  float local_98;
  float local_94;
  int local_90;
  undefined4 local_8c;
  undefined4 local_88;
  undefined4 local_84;
  undefined1 local_80 [4];
  undefined4 local_7c;
  undefined4 local_78;
  undefined4 local_74;
  undefined4 local_38;
  undefined4 local_34;
  int local_30;
  undefined4 local_2c;
  undefined4 local_28;
  int *local_24;
  undefined1 local_20 [12];
  float local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094e188;
  local_10 = ExceptionList;
  uVar3 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_8c = 0;
  local_88 = 0;
  local_84 = 0;
  local_2c = 0x7f000000;
  local_14 = _DAT_00993900;
  local_38 = 1;
  if ((float)param_3[2 = byte +0x8] <= (float)param_2[2 = byte +0x8]) {
    local_28 = param_3[2 = byte +0x8];
    local_34 = param_2[2 = byte +0x8];
  }
  else {
    local_28 = param_2[2 = byte +0x8];
    local_34 = param_3[2 = byte +0x8];
  }
  FUN_0084a640(0);
  local_8 = 0;
  local_24 = (int *)FUN_0051c0a0(param_6);
  if ((local_24 != (int *)0x0) && (iVar4 = (**(code **)(*local_24 + 0x10))(uVar3), iVar4 != 0)) {
    FUN_0083ea60(local_24);
  }
  local_24 = (int *)FUN_0051c0a0(param_7);
  if ((local_24 != (int *)0x0) && (iVar4 = (**(code **)(*local_24 + 0x10))(), iVar4 != 0)) {
    FUN_0083ea60(local_24);
  }
  FUN_0052be60();
  local_30 = FUN_0052d130(param_2,param_3,local_28,local_34,_DAT_0098bd74,0x7f000000,param_5,1,
                          local_20,param_8,local_80,0);
  if (local_30 == 0) {
    *param_4 = local_7c;
    param_4[1 = byte +0x4] = local_78;
    param_4[2 = byte +0x8] = local_74;
    local_38 = 0;
    if (param_9 == 0) goto LAB_0052be30;
  }
  for (local_90 = 0; local_90 < *(int *)(param_1 + 0x250); local_90 = local_90 + 1) {
    iVar4 = FUN_005ff950(*param_2,param_2[1 = byte +0x4],param_2[2 = byte +0x8],*param_3,param_3[1 = byte +0x4],param_3[2 = byte +0x8],&local_8c,
                         &local_2c);
    if (iVar4 == 0) {
      FUN_004452b0(&local_9c,&local_8c,param_2);
      fVar1 = local_9c * local_9c + local_98 * local_98 + local_94 * local_94;
/* ... 16 more lines truncated by --max-lines */
```

### decoy: 0x0052be60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0052be60: 16-byte aligned: YES
  0052be60  55             PUSH EBP
  0052be61  8B EC          MOV EBP,ESP
  0052be63  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0052be60
string literals (0):
direct callees (0):
struct offsets touched (top 4):
  +0x4 [dec 4]  x5  via EAX,ECX,EDX
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0x40 [dec 64]  x1  via ECX
  +0x44 [dec 68]  x1  via EAX
```

## Card B0-8-30

GOG function: CSWSEffectListHandler::OnRemoveDamageImmunityIncrease  gog 0x0073da90  callconv=(blank)  param_size_bytes=8  gog_size=313  gog_retn=8
Candidate Steam address: 0x0059b1a0 (size 313, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0059b3e0:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0073da20 CSWSEffectListHandler::OnApplyDamageImmunityIncrease -> 0x0059b130  gate_class=REVIEW
- GOG-order neighbour after:  0x0073dbd0 CSWSEffectListHandler::OnApplyDamageImmunityDecrease -> 0x0059b2e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006a9ed0 CSWSObject::SetDamageImmunity -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0xb4*} | Steam {0x4*,0x8*,0xb4*}
imm sets (* = in both, 5 common): GOG {0x8*,0x10*,0x11*,0x2c*,0x148*} | Steam {0x8*,0x10*,0x11*,0x2c*,0x148*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0059b1a0: 16-byte aligned: YES
  0059b1a0  55             PUSH EBP
  0059b1a1  8B EC          MOV EBP,ESP
  0059b1a3  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059b1a0
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x005431F0  (unnamed)
struct offsets touched (top 3):
  +0x8 [dec 8]  x3  via EDX
  +0x4 [dec 4]  x1  via ECX
  +0xB4 [dec 180]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0059b1a0 FUN_0059b1a0 size=313 ret=8 callers=0 callees=2

undefined4 FUN_0059b1a0(int *param_1,int param_2)

{
  int iVar1;
  char cVar2;
  char cVar3;
  char cVar4;
  uint uVar5;
  uint uVar6;
  undefined4 extraout_var;
  undefined4 uVar7;
  int local_8;
  
  if (param_2 != 0) {
    uVar5 = FUN_005e4e60(0);
    uVar7 = extraout_var;
    cVar2 = FUN_005e4e60(1);
    (**(code **)(*param_1 + 0xb4))(uVar5 & 0xffff,uVar7);
    cVar4 = '\0';
    for (local_8 = 0; local_8 < param_1[0x53 = byte +0x14c]; local_8 = local_8 + 1) {
      iVar1 = *(int *)(param_1[0x52 = byte +0x148] + local_8 * 4);
      if (*(short *)(iVar1 + 8) == 0x10) {
        uVar6 = FUN_005e4e60(0);
        if (uVar6 != uVar5) goto LAB_0059b253;
LAB_0059b276:
        if (*(short *)(iVar1 + 8) == 0x10) {
          cVar3 = FUN_005e4e60(1);
          cVar4 = cVar3 + cVar4;
        }
        else {
          cVar3 = FUN_005e4e60(1);
          cVar4 = cVar4 - cVar3;
        }
      }
      else {
LAB_0059b253:
        if (*(short *)(iVar1 + 8) == 0x11) {
          uVar6 = FUN_005e4e60(0);
          if (uVar6 == uVar5) goto LAB_0059b276;
        }
      }
    }
    FUN_005431f0(uVar5 & 0xffff,cVar4 - cVar2);
  }
  return 1;
}
```

### decoy: 0x0059b3e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059b3e0: 16-byte aligned: YES
  0059b3e0  55             PUSH EBP
  0059b3e1  8B EC          MOV EBP,ESP
  0059b3e3  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059b3e0
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x005431F0  (unnamed)
struct offsets touched (top 3):
  +0x8 [dec 8]  x3  via EDX
  +0x4 [dec 4]  x1  via ECX
  +0xB0 [dec 176]  x1  via EDX
```

## Card B0-8-31

GOG function: CSWSEffectListHandler::OnRemoveForceSight  gog 0x007362f0  callconv=(blank)  param_size_bytes=0  gog_size=20  gog_retn=0
Candidate Steam address: 0x00593a10 (size 20, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00593960:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00736240 CSWSEffectListHandler::Destructor_2 -> 0x00593960  gate_class=REVIEW
- GOG-order neighbour after:  0x00736310 CSWSEffectListHandler::InitializeEffects -> 0x00593a30  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
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
prologue for 0x00593a10: 16-byte aligned: YES
  00593a10  55             PUSH EBP
  00593a11  8B EC          MOV EBP,ESP
  00593a13  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00593a10
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00593a10 FUN_00593a10 size=20 ret=0 callers=1 callees=0

void __fastcall FUN_00593a10(undefined4 *param_1)

{
  *param_1 = CGameEffectApplierRemover::vftable;
  return;
}
```

### decoy: 0x00593960 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00593960: 16-byte aligned: YES
  00593960  55             PUSH EBP
  00593961  8B EC          MOV EBP,ESP
  00593963  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00593960
string literals (0):
direct callees (1):
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x4 [dec 4]  x3  via ECX,EDX
  +0x8 [dec 8]  x3  via EAX,ECX
```

## Card B0-8-32

GOG function: CSWSEffectListHandler::OnApplyMovementSpeedIncrease  gog 0x00741260  callconv=(blank)  param_size_bytes=8  gog_size=145  gog_retn=12
Candidate Steam address: 0x0059eb80 (size 226, retn 12)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSEffectListHandler::OnApplyMovementSpeedDecrease
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.30 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0059ea10:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00741150 CSWSEffectListHandler::OnRemoveSetStateInternal -> 0x0059e860  gate_class=REVIEW
- GOG-order neighbour after:  0x00741300 CSWSEffectListHandler::OnRemoveMovementSpeedIncrease -> 0x0059ea10  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0068f140 CSWSCreature::GetMovementRateFactor -> Steam ?  in candidate callees: n/a
  0x0068f1d0 CSWSCreature::SetMovementRateFactor -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x30*} | Steam {0x8,0x18,0x30*,0xfc,0x1198}
imm sets (* = in both, 2 common): GOG {0xc*,0x14,0x64*} | Steam {0xc*,0x18,0x28,0x64*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x0059eb80: 16-byte aligned: YES
  0059eb80  55             PUSH EBP
  0059eb81  8B EC          MOV EBP,ESP
  0059eb83  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059eb80
string literals (0):
direct callees (5):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006BC570  CSWSCreatureStats::GetEffectImmunity
  0x005E4E60  CGameEffect::GetInteger
  0x00576610  CSWSCreature::GetMovementRateFactor
  0x005766A0  CSWSCreature::SetMovementRateFactor
struct offsets touched (top 5):
  +0x30 [dec 48]  x2  via EDX
  +0x8 [dec 8]  x1  via EAX
  +0x18 [dec 24]  x1  via ECX
  +0xFC [dec 252]  x1  via EAX
  +0x1198 [dec 4504]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0059eb80 FUN_0059eb80 size=226 ret=12 callers=0 callees=5

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_0059eb80(int *param_1,int param_2)

{
  float fVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  float10 fVar5;
  
  iVar2 = (**(code **)(*param_1 + 0x30))();
  if (iVar2 != 0) {
    iVar2 = (**(code **)(*param_1 + 0x30))();
    uVar3 = FUN_0051c100(*(undefined4 *)(param_2 + 0x18));
    iVar4 = FUN_006bc570(0x18,uVar3);
    if (iVar4 != 0) {
      return 1;
    }
    iVar4 = FUN_005e4e60(0);
    if ((99 < iVar4) || (*(int *)(iVar2 + 0xfc) != 0)) {
      return 1;
    }
    iVar2 = FUN_005e4e60(0);
    fVar1 = (float)_DAT_0098ba38;
    fVar5 = (float10)FUN_00576610(0);
    FUN_005766a0((float)(fVar5 * (float10)(1.0 - (float)iVar2 / fVar1)));
  }
  return 0;
}
```

### decoy: 0x0059ea10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059ea10: 16-byte aligned: YES
  0059ea10  55             PUSH EBP
  0059ea11  8B EC          MOV EBP,ESP
  0059ea13  83 EC 44       SUB ESP,0x44
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059ea10
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x005766A0  CSWSCreature::SetMovementRateFactor
struct offsets touched (top 3):
  +0x4 [dec 4]  x3  via EDX
  +0x8 [dec 8]  x3  via ECX
  +0x30 [dec 48]  x3  via EDX
```

## Card B0-8-33

GOG function: CGameEffect::GetFloat  gog 0x00734b60  callconv=(blank)  param_size_bytes=4  gog_size=23  gog_retn=4
Candidate Steam address: 0x005e4ea0 (size 23, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005e4ec0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00734b40 CGameEffect::SetInteger -> 0x005e4e80  gate_class=REVIEW
- GOG-order neighbour after:  0x00734b80 CGameEffect::SetFloat -> 0x005e4ec0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x38*} | Steam {0x38*}
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
prologue for 0x005e4ea0: 16-byte aligned: YES
  005e4ea0  55             PUSH EBP
  005e4ea1  8B EC          MOV EBP,ESP
  005e4ea3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005e4ea0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005e4ea0 FUN_005e4ea0 size=23 ret=4 callers=10 callees=0
THUNK -> unknown (Ghidra may omit __thiscall on thunks; ECX is passed through)

float10 __thiscall FUN_005e4ea0(int param_1,int param_2)

{
  return (float10)*(float *)(param_1 + 0x38 + param_2 * 4);
}
```

### decoy: 0x005e4ec0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005e4ec0: 16-byte aligned: YES
  005e4ec0  55             PUSH EBP
  005e4ec1  8B EC          MOV EBP,ESP
  005e4ec3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005e4ec0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

## Card B0-8-34

GOG function: CSWSEffectListHandler::OnApplyAttackDecrease  gog 0x0073d330  callconv=(blank)  param_size_bytes=8  gog_size=210  gog_retn=12
Candidate Steam address: 0x0059aa90 (size 210, retn 12)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0059abb0:STR0/CALLEE0/SIG0.517/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0073d2d0 CSWSEffectListHandler::OnApplyAttackIncrease -> 0x0059a9e0  gate_class=REVIEW
- GOG-order neighbour after:  0x0073d410 CSWSEffectListHandler::OnApplyDamageIncrease -> 0x0059ab70  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x006f89f0 CSWSCreatureStats::GetEffectImmunity -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x8*,0x18*,0x30*,0xfc*,0x384*,0x1198*} | Steam {0x8*,0x18*,0x30*,0xfc*,0x384*,0x1198*}
imm sets (* = in both, 3 common): GOG {0xc*,0x14*,0x24*} | Steam {0xc*,0x14*,0x24*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x0059aa90: 16-byte aligned: YES
  0059aa90  55             PUSH EBP
  0059aa91  8B EC          MOV EBP,ESP
  0059aa93  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059aa90
string literals (0):
direct callees (3):
  0x005E4E60  CGameEffect::GetInteger
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006BC570  CSWSCreatureStats::GetEffectImmunity
struct offsets touched (top 6):
  +0x30 [dec 48]  x3  via EAX,EDX
  +0x8 [dec 8]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
  +0xFC [dec 252]  x1  via ECX
  +0x384 [dec 900]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0059aa90 FUN_0059aa90 size=210 ret=12 callers=0 callees=3

undefined4 __thiscall FUN_0059aa90(undefined4 param_1,int *param_2,int param_3)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = (**(code **)(*param_2 + 0x30))(param_1);
  if ((iVar1 != 0) && (param_3 != 0)) {
    iVar1 = FUN_005e4e60(0);
    if ((iVar1 < 1) || (param_2[0x3f = byte +0xfc] != 0)) {
      return 1;
    }
    uVar2 = FUN_0051c100(*(undefined4 *)(param_3 + 0x18));
    (**(code **)(*param_2 + 0x30))();
    iVar1 = FUN_006bc570(0x14,uVar2);
    if (iVar1 != 0) {
      return 1;
    }
    iVar1 = (**(code **)(*param_2 + 0x30))();
    *(undefined4 *)(iVar1 + 0x384 [dec 900]) = 1;
  }
  return 0;
}
```

### decoy: 0x0059abb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059abb0: 16-byte aligned: YES
  0059abb0  55             PUSH EBP
  0059abb1  8B EC          MOV EBP,ESP
  0059abb3  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059abb0
string literals (0):
direct callees (2):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006BC570  CSWSCreatureStats::GetEffectImmunity
struct offsets touched (top 5):
  +0x30 [dec 48]  x3  via EDX
  +0x8 [dec 8]  x1  via EAX
  +0x18 [dec 24]  x1  via ECX
  +0x384 [dec 900]  x1  via ECX
  +0x1198 [dec 4504]  x1  via ECX
```

## Card B0-8-35

GOG function: CSWSArea::GetClientArea_2  gog 0x00722ee0  callconv=(blank)  param_size_bytes=4  gog_size=51  gog_retn=4
Candidate Steam address: 0x0052de00 (size 51, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch5_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0052de40:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00722de0 CSWSArea::AwardStealthXP -> 0x0052dd00  gate_class=AUTO
- GOG-order neighbour after:  0x00722f60 CSWSArea::EnforceNoPCHeadCloning -> 0x0052de80  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x2e8*,0x2ec*} | Steam {0x2e8*,0x2ec*}
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
prologue for 0x0052de00: 16-byte aligned: YES
  0052de00  55             PUSH EBP
  0052de01  8B EC          MOV EBP,ESP
  0052de03  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0052de00
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x2EC [dec 748]  x2  via EAX,EDX
  +0x2E8 [dec 744]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0052de00 FUN_0052de00 size=51 ret=4 callers=3 callees=0

void __thiscall FUN_0052de00(int param_1,uint param_2)

{
  if (param_2 < *(uint *)(param_1 + 0x2ec)) {
    *(uint *)(param_1 + 0x2ec) = param_2;
  }
  *(uint *)(param_1 + 0x2e8) = param_2;
  return;
}
```

### decoy: 0x0052de40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0052de40: 16-byte aligned: YES
  0052de40  55             PUSH EBP
  0052de41  8B EC          MOV EBP,ESP
  0052de43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0052de40
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x2E8 [dec 744]  x2  via EAX,EDX
  +0x2EC [dec 748]  x1  via ECX
```

## Card B0-8-36

GOG function: CGameEffect::GetInteger  gog 0x00734b20  callconv=(blank)  param_size_bytes=4  gog_size=25  gog_retn=4
Candidate Steam address: 0x005e4e60 (size 25, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005e4e80:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00734a90 CGameEffect::SetNumIntegers -> 0x005e4dd0  gate_class=REVIEW
- GOG-order neighbour after:  0x00734b40 CGameEffect::SetInteger -> 0x005e4e80  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x34*} | Steam {0x34*}
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
prologue for 0x005e4e60: 16-byte aligned: YES
  005e4e60  55             PUSH EBP
  005e4e61  8B EC          MOV EBP,ESP
  005e4e63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005e4e60
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x34 [dec 52]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005e4e60 FUN_005e4e60 size=25 ret=4 callers=154 callees=0

undefined4 __thiscall FUN_005e4e60(int param_1,int param_2)

{
  return *(undefined4 *)(*(int *)(param_1 + 0x34) + param_2 * 4);
}
```

### decoy: 0x005e4e80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005e4e80: 16-byte aligned: YES
  005e4e80  55             PUSH EBP
  005e4e81  8B EC          MOV EBP,ESP
  005e4e83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005e4e80
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x34 [dec 52]  x1  via EAX
```

## Card B0-8-37

GOG function: CSWSEffectListHandler::OnRemoveACDecrease  gog 0x0073d020  callconv=(blank)  param_size_bytes=8  gog_size=662  gog_retn=8
Candidate Steam address: 0x0059a730 (size 662, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0059a4b0:STR0/CALLEE0/SIG0.483/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0073cda0 CSWSEffectListHandler::OnApplyACDecrease -> 0x0059a4b0  gate_class=AUTO
- GOG-order neighbour after:  0x0073d2d0 CSWSEffectListHandler::OnApplyAttackIncrease -> 0x0059a9e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 11 common): GOG {0x4*,0x8*,0x30*,0xfd*,0xff*,0x101*,0x103*,0x105*,0x122*,0x13a*,0x1198*} | Steam {0x4*,0x8*,0x30*,0xfd*,0xff*,0x101*,0x103*,0x105*,0x122*,0x13a*,0x1198*}
imm sets (* = in both, 7 common): GOG {0x2*,0x3*,0x4*,0x8*,0x31*,0x70*,0x148*} | Steam {0x2*,0x3*,0x4*,0x8*,0x31*,0x70*,0x148*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0059a730: 16-byte aligned: YES
  0059a730  55             PUSH EBP
  0059a731  8B EC          MOV EBP,ESP
  0059a733  83 EC 70       SUB ESP,0x70
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059a730
string literals (0):
direct callees (1):
  0x005E4E60  CGameEffect::GetInteger
struct offsets touched (top 10):
  +0x1198 [dec 4504]  x7  via EAX,ECX,EDX
  +0x30 [dec 48]  x2  via EDX
  +0x105 [dec 261]  x2  via EAX,EDX
  +0x122 [dec 290]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EDX
  +0xFD [dec 253]  x1  via ECX
  +0xFF [dec 255]  x1  via EAX
  +0x101 [dec 257]  x1  via EDX
  +0x103 [dec 259]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0059a730 FUN_0059a730 size=662 ret=8 callers=0 callees=1

undefined4 FUN_0059a730(int *param_1,int param_2)

{
  byte bVar1;
  char cVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  int iVar6;
  int iVar7;
  int local_28;
  uint local_20;
  
  iVar3 = (**(code **)(*param_1 + 0x30))();
  if ((iVar3 != 0) && (iVar3 = (**(code **)(*param_1 + 0x30))(), param_2 != 0)) {
    iVar4 = FUN_005e4e60(0);
    cVar2 = FUN_005e4e60(1);
    uVar5 = FUN_005e4e60(2);
    iVar6 = FUN_005e4e60(3);
    iVar7 = FUN_005e4e60(4);
    local_20 = (uint)*(ushort *)(*(int *)(iVar3 + 0x1198) + 0x13a);
    if ((uVar5 == *(byte *)(DAT_00a1b4d0 + 0x122)) && ((iVar6 == 0 && (iVar7 == 0)))) {
      if (iVar4 == 0) {
        *(char *)(*(int *)(iVar3 + 0x1198) + 0x105) =
             *(char *)(*(int *)(iVar3 + 0x1198) + 0x105) - cVar2;
      }
      else {
        local_28 = 0;
        while (((int)local_20 < *(int *)(iVar3 + 0x14c) &&
               (*(short *)(*(int *)(*(int *)(iVar3 + 0x148) + local_20 * 4) + 8) == 0x31))) {
          if ((*(int *)(*(int *)(iVar3 + 0x148) + local_20 * 4) != param_2) &&
             (((iVar6 = FUN_005e4e60(0), iVar6 == iVar4 &&
               (bVar1 = *(byte *)(DAT_00a1b4d0 + 0x122), uVar5 = FUN_005e4e60(2), uVar5 == bVar1))
              && (iVar6 = FUN_005e4e60(1), local_28 < iVar6)))) {
            local_28 = FUN_005e4e60(1);
          }
          local_20 = local_20 + 1;
        }
        switch(iVar4) {
        case 1:
          *(undefined1 *)(*(int *)(iVar3 + 0x1198) + 0x103) = (undefined1)local_28;
          break;
        case 2:
          *(undefined1 *)(*(int *)(iVar3 + 0x1198) + 0xfd) = (undefined1)local_28;
          break;
        case 3:
          *(undefined1 *)(*(int *)(iVar3 + 0x1198) + 0x101) = (undefined1)local_28;
          break;
        case 4:
          *(undefined1 *)(*(int *)(iVar3 + 0x1198) + 0xff) = (undefined1)local_28;
        }
      }
    }
  }
  return 1;
}
```

### decoy: 0x0059a4b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059a4b0: 16-byte aligned: YES
  0059a4b0  55             PUSH EBP
  0059a4b1  8B EC          MOV EBP,ESP
  0059a4b3  83 EC 78       SUB ESP,0x78
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059a4b0
string literals (0):
direct callees (3):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006BC570  CSWSCreatureStats::GetEffectImmunity
  0x005E4E60  CGameEffect::GetInteger
struct offsets touched (top 10):
  +0x1198 [dec 4504]  x11  via EAX,ECX,EDX
  +0x30 [dec 48]  x2  via EDX
  +0xFD [dec 253]  x2  via ECX,EDX
  +0xFF [dec 255]  x2  via ECX,EDX
  +0x101 [dec 257]  x2  via ECX,EDX
  +0x103 [dec 259]  x2  via ECX,EDX
  +0x105 [dec 261]  x2  via EAX,EDX
  +0x8 [dec 8]  x1  via EAX
  +0x18 [dec 24]  x1  via ECX
  +0xFC [dec 252]  x1  via EAX
```

## Card B0-8-38

GOG function: CSWSEffectListHandler::OnRemoveDamageDecrease  gog 0x0073d4f0  callconv=(blank)  param_size_bytes=4  gog_size=66  gog_retn=8
Candidate Steam address: 0x0059aa40 (size 66, retn 8)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0059ab70:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0073d450 CSWSEffectListHandler::OnApplyDamageDecrease -> 0x0059abb0  gate_class=REVIEW
- GOG-order neighbour after:  0x0073d540 CSWSEffectListHandler::OnApplyTemporaryHitpoints -> 0x0059ac50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x30*,0x384*} | Steam {0x30*,0x384*}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 8)
```

### prologue
```
prologue for 0x0059aa40: 16-byte aligned: YES
  0059aa40  55             PUSH EBP
  0059aa41  8B EC          MOV EBP,ESP
  0059aa43  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059aa40
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x30 [dec 48]  x2  via EDX
  +0x384 [dec 900]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0059aa40 FUN_0059aa40 size=66 ret=8 callers=0 callees=0

undefined4 __thiscall FUN_0059aa40(undefined4 param_1,int *param_2)

{
  int iVar1;
  
  iVar1 = (**(code **)(*param_2 + 0x30))(param_1);
  if (iVar1 != 0) {
    iVar1 = (**(code **)(*param_2 + 0x30))();
    *(undefined4 *)(iVar1 + 0x384 [dec 900]) = 1;
  }
  return 1;
}
```

### decoy: 0x0059ab70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059ab70: 16-byte aligned: YES
  0059ab70  55             PUSH EBP
  0059ab71  8B EC          MOV EBP,ESP
  0059ab73  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059ab70
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x30 [dec 48]  x2  via EDX
  +0x384 [dec 900]  x1  via ECX
```

## Card B0-8-39

GOG function: CSWSEffectListHandler::OnApplyFury  gog 0x00737830  callconv=(blank)  param_size_bytes=8  gog_size=215  gog_retn=12
Candidate Steam address: 0x00594f50 (size 215, retn 12)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005950b0:STR0/CALLEE0/SIG0.336/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007370f0 CSWSEffectListHandler::OnApplyForceShield -> 0x00594810  gate_class=REVIEW
- GOG-order neighbour after:  0x00737990 CSWSEffectListHandler::OnRemoveFury -> 0x005950b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x1c*,0x30*,0x120*,0x124*,0x1198*} | Steam {0x1c*,0x30*,0x120*,0x124*,0x1198*}
imm sets (* = in both, 7 common): GOG {0x7*,0x8*,0x9*,0xc*,0x20*,0x6d*,0xa4*} | Steam {0x7*,0x8*,0x9*,0xc*,0x20*,0x6d*,0xa4*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x00594f50: 16-byte aligned: YES
  00594f50  55             PUSH EBP
  00594f51  8B EC          MOV EBP,ESP
  00594f53  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00594f50
string literals (0):
direct callees (0):
struct offsets touched (top 6):
  +0x1198 [dec 4504]  x4  via ECX
  +0x124 [dec 292]  x3  via EAX
  +0x30 [dec 48]  x2  via EDX
  +0x1C [dec 28]  x1  via EDX
  +0x120 [dec 288]  x1  via EAX
  +0x595038 [dec 5853240]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00594f50 FUN_00594f50 size=215 ret=12 callers=0 callees=0

undefined4 FUN_00594f50(int *param_1,int param_2)

{
  int iVar1;
  
  if ((param_1 != (int *)0x0) && (iVar1 = (**(code **)(*param_1 + 0x30))(), iVar1 != 0)) {
    iVar1 = (**(code **)(*param_1 + 0x30))();
    *(undefined1 *)(*(int *)(iVar1 + 0x1198) + 0x120) = 0;
    switch(*(undefined4 *)(param_2 + 0x1c)) {
    case 0xa4:
    case 0x10f:
      *(undefined4 *)(*(int *)(iVar1 + 0x1198) + 0x124) = 7;
      break;
    case 0xa5:
    case 0x110:
      *(undefined4 *)(*(int *)(iVar1 + 0x1198) + 0x124) = 8;
      break;
    case 0xa6:
    case 0x111:
      *(undefined4 *)(*(int *)(iVar1 + 0x1198) + 0x124) = 9;
    }
  }
  return 0;
}
```

### decoy: 0x005950b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005950b0: 16-byte aligned: YES
  005950b0  55             PUSH EBP
  005950b1  8B EC          MOV EBP,ESP
  005950b3  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005950b0
string literals (0):
direct callees (0):
struct offsets touched (top 6):
  +0x30 [dec 48]  x2  via EDX
  +0x1198 [dec 4504]  x2  via EDX
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
  +0x120 [dec 288]  x1  via ECX
  +0x124 [dec 292]  x1  via ECX
```

## Card B0-8-40

GOG function: CSWSEffectListHandler::OnApplyDamageImmunityDecrease  gog 0x0073dbd0  callconv=(blank)  param_size_bytes=8  gog_size=241  gog_retn=12
Candidate Steam address: 0x0059b2e0 (size 241, retn 12)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0059b1a0:STR0/CALLEE0/SIG0.143/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0073da90 CSWSEffectListHandler::OnRemoveDamageImmunityIncrease -> 0x0059b1a0  gate_class=REVIEW
- GOG-order neighbour after:  0x0073dcd0 CSWSEffectListHandler::OnRemoveDamageImmunityDecrease -> 0x0059b3e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x006a9ed0 CSWSObject::SetDamageImmunity -> Steam ?  in candidate callees: n/a
  0x006f89f0 CSWSCreatureStats::GetEffectImmunity -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x8*,0x18*,0x30*,0xb4*,0xfc*,0x1198*} | Steam {0x8*,0x18*,0x30*,0xb4*,0xfc*,0x1198*}
imm sets (* = in both, 3 common): GOG {0xc*,0x16*,0x24*} | Steam {0xc*,0x16*,0x24*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x0059b2e0: 16-byte aligned: YES
  0059b2e0  55             PUSH EBP
  0059b2e1  8B EC          MOV EBP,ESP
  0059b2e3  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059b2e0
string literals (0):
direct callees (4):
  0x005E4E60  CGameEffect::GetInteger
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006BC570  CSWSCreatureStats::GetEffectImmunity
  0x005431F0  (unnamed)
struct offsets touched (top 6):
  +0x30 [dec 48]  x2  via EAX
  +0x8 [dec 8]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
  +0xB4 [dec 180]  x1  via EDX
  +0xFC [dec 252]  x1  via EAX
  +0x1198 [dec 4504]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0059b2e0 FUN_0059b2e0 size=241 ret=12 callers=0 callees=4

undefined4 FUN_0059b2e0(int *param_1,int param_2)

{
  char cVar1;
  undefined2 uVar2;
  int iVar3;
  int iVar4;
  undefined4 uVar5;
  undefined4 extraout_var;
  
  if (param_2 != 0) {
    uVar2 = FUN_005e4e60(0);
    uVar5 = extraout_var;
    iVar3 = FUN_005e4e60(1);
    if ((iVar3 < 0) || (param_1[0x3f = byte +0xfc] != 0)) {
      return 1;
    }
    iVar4 = (**(code **)(*param_1 + 0x30))(uVar5);
    if (iVar4 != 0) {
      uVar5 = FUN_0051c100(*(undefined4 *)(param_2 + 0x18));
      (**(code **)(*param_1 + 0x30))();
      iVar4 = FUN_006bc570(0x16,uVar5);
      if (iVar4 != 0) {
        return 1;
      }
    }
    cVar1 = (**(code **)(*param_1 + 0xb4))(uVar2);
    FUN_005431f0(uVar2,cVar1 - iVar3 & 0xff);
  }
  return 0;
}
```

### decoy: 0x0059b1a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059b1a0: 16-byte aligned: YES
  0059b1a0  55             PUSH EBP
  0059b1a1  8B EC          MOV EBP,ESP
  0059b1a3  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059b1a0
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x005431F0  (unnamed)
struct offsets touched (top 3):
  +0x8 [dec 8]  x3  via EDX
  +0x4 [dec 4]  x1  via ECX
  +0xB4 [dec 180]  x1  via EDX
```

## Card B0-8-41

GOG function: CSWSArea::SetMaxStealthXP  gog 0x007184d0  callconv=(blank)  param_size_bytes=0  gog_size=52  gog_retn=0
Candidate Steam address: 0x00523360 (size 52, retn 0)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSArea::DecrementPlayersInArea
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00523320:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00718460 CSWSArea::GetFirstObjectInArea -> 0x005232b0  gate_class=REVIEW
- GOG-order neighbour after:  0x00718510 CSWSArea::DecrementPlayersInArea -> 0x00523360  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00718550 CSWSArea::IncreaseAILevelPriority -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x12c*} | Steam {0x12c*}
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
prologue for 0x00523360: 16-byte aligned: YES
  00523360  55             PUSH EBP
  00523361  8B EC          MOV EBP,ESP
  00523363  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00523360
string literals (0):
direct callees (1):
  0x005235A0  CSWSArea::DecreaseAILevelPriority
struct offsets touched (top 1):
  +0x12C [dec 300]  x3  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00523360 FUN_00523360 size=52 ret=0 callers=2 callees=1

void __fastcall FUN_00523360(int param_1)

{
  *(int *)(param_1 + 0x12c [dec 300]) = *(int *)(param_1 + 0x12c [dec 300]) + -1;
  if (*(int *)(param_1 + 0x12c [dec 300]) == 0) {
    FUN_005235a0();
  }
  return;
}
```

### decoy: 0x00523320 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00523320: 16-byte aligned: YES
  00523320  55             PUSH EBP
  00523321  8B EC          MOV EBP,ESP
  00523323  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00523320
string literals (0):
direct callees (1):
  0x005233A0  CSWSArea::IncreaseAILevelPriority
struct offsets touched (top 1):
  +0x12C [dec 300]  x3  via EAX,ECX
```

## Card B0-8-42

GOG function: CGameEffect::Destructor  gog 0x00734960  callconv=(blank)  param_size_bytes=0  gog_size=300  gog_retn=0
Candidate Steam address: 0x005e4ca0 (size 300, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005e4ac0:STR0/CALLEE0/SIG0.31/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00734780 CGameEffect::Constructor_2 -> 0x005e4ac0  gate_class=REVIEW
- GOG-order neighbour after:  0x00734a90 CGameEffect::SetNumIntegers -> 0x005e4dd0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00734960 CGameEffect::Destructor -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  0x009217f0 Global::`eh_vector_destructor_iterator' -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x28*,0x2c*,0x34*} | Steam {0x28*,0x2c*,0x34*}
imm sets (* = in both, 5 common): GOG {0x4*,0x6*,0x8*,0x20*,0x48*} | Steam {0x4*,0x6*,0x8*,0x20*,0x48*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x005e4ca0: 16-byte aligned: YES
  005e4ca0  55             PUSH EBP
  005e4ca1  8B EC          MOV EBP,ESP
  005e4ca3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005e4ca0
string literals (0):
direct callees (3):
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
  0x00919BCC  `eh vector destructor iterator` (CRT)
struct offsets touched (top 3):
  +0x28 [dec 40]  x3  via EAX,ECX
  +0x2C [dec 44]  x3  via EAX,EDX
  +0x34 [dec 52]  x3  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005e4ca0 FUN_005e4ca0 size=300 ret=0 callers=104 callees=3

void __fastcall FUN_005e4ca0(int param_1)

{
  int iVar1;
  uint uVar2;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_00953e56;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_8 = 0;
  if (*(int *)(param_1 + 0x28) != 0) {
    iVar1 = *(int *)(param_1 + 0x28);
    if (iVar1 != 0) {
      FUN_005e4ca0(uVar2);
      FUN_009196fd(iVar1);
    }
    *(undefined4 *)(param_1 + 0x28) = 0;
  }
  if (*(int *)(param_1 + 0x2c) != 0) {
    iVar1 = *(int *)(param_1 + 0x2c);
    if (iVar1 != 0) {
      FUN_005e4ca0(uVar2);
      FUN_009196fd(iVar1);
    }
    *(undefined4 *)(param_1 + 0x2c) = 0;
  }
  if (*(int *)(param_1 + 0x34) != 0) {
    FUN_009196fd(*(undefined4 *)(param_1 + 0x34));
    *(undefined4 *)(param_1 + 0x34) = 0;
  }
  local_8 = 0xffffffff;
  _eh_vector_destructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable]((void *)(param_1 + 0x48),8,6,FUN_00733780);
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x005e4ac0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005e4ac0: 16-byte aligned: YES
  005e4ac0  55             PUSH EBP
  005e4ac1  8B EC          MOV EBP,ESP
  005e4ac3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005e4ac0
string literals (0):
direct callees (4):
  0x0091D67E  (unnamed)
  0x005E4DD0  CGameEffect::SetNumIntegers
  0x0091C630  (unnamed)
  0x007338D0  CExoString::operator=_2
struct offsets touched (top 10):
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
  +0xA [dec 10]  x3  via EAX,ECX,EDX
  +0xC [dec 12]  x3  via ECX,EDX
  +0x10 [dec 16]  x3  via EAX,ECX,EDX
  +0x14 [dec 20]  x3  via EAX,ECX
  +0x18 [dec 24]  x3  via EAX,ECX
  +0x1C [dec 28]  x3  via EAX,ECX,EDX
  +0x8 [dec 8]  x1  via ECX
  +0x20 [dec 32]  x1  via EDX
  +0x24 [dec 36]  x1  via EAX
```

## Card B0-8-43

GOG function: CSWSArea::Constructor_2  gog 0x00720dc0  callconv=(blank)  param_size_bytes=0  gog_size=352  gog_retn=None
Candidate Steam address: 0x0052bb60 (size 767, retn 32)
Candidate sources: near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: n/a | G5 thunk: none | G6 dup: review:held_by=CSWSArea::ClearLineOfSight
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.57 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.57
- best decoy (neighbour/other candidate) scores: 0x0052b9c0:STR0/CALLEE0/SIG0.236/SRV0/VT0
- flags: retn_meta_mismatch
- GOG-order neighbour before: 0x00720d10 CSWSArea::ClearLineOfSight -> 0x0052bb60  gate_class=REVIEW
- GOG-order neighbour after:  0x00720f20 CSWSArea::Destructor_3 -> 0x0052bb60  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00422ca0 CWalkHitInfo::Constructor -> Steam ?  in candidate callees: n/a
  0x004c7640 CExoArrayList::Add -> Steam ?  in candidate callees: n/a
  0x0064bac0 CServerExoApp::GetGameObject -> Steam 0x0051c0a0  in candidate callees: yes
  0x00722210 CSWSArea::NoNonWalkPolysDetailed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0x10*,0x250*} | Steam {0x4*,0x8*,0x10*,0x250*,0x254}
imm sets (* = in both, 2 common): GOG {0xc*,0x7f000000*} | Steam {0xc*,0x20,0x41,0x4c,0xc0,0x7f000000*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=32 ; asm RET = 32
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 32)
```

### prologue
```
prologue for 0x0052bb60: 16-byte aligned: YES
  0052bb60  55             PUSH EBP
  0052bb61  8B EC          MOV EBP,ESP
  0052bb63  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0052bb60
string literals (0):
direct callees (8):
  0x0084A640  (unnamed)
  0x0051C0A0  CServerExoApp::GetGameObject
  0x0083EA60  CExoArrayList<uint32>::Add
  0x0052BE60  CSWSArea::Constructor_2
  0x0052D130  CSWSArea::NoNonWalkPolysDetailed
  0x005FF950  (unnamed)
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0064DDB0  Free(ptr*) delete-and-zero helper
struct offsets touched (top 5):
  +0x8 [dec 8]  x14  via EAX,ECX,EDX
  +0x4 [dec 4]  x6  via EAX,ECX,EDX
  +0x10 [dec 16]  x2  via EAX,EDX
  +0x250 [dec 592]  x1  via ECX
  +0x254 [dec 596]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0052bb60 FUN_0052bb60 size=767 ret=32 callers=20 callees=8

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall
FUN_0052bb60(int param_1,undefined4 *param_2,undefined4 *param_3,undefined4 *param_4,
            undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8,int param_9)

{
  float fVar1;
  undefined4 uVar2;
  uint uVar3;
  int iVar4;
  float local_9c;
  float local_98;
  float local_94;
  int local_90;
  undefined4 local_8c;
  undefined4 local_88;
  undefined4 local_84;
  undefined1 local_80 [4];
  undefined4 local_7c;
  undefined4 local_78;
  undefined4 local_74;
  undefined4 local_38;
  undefined4 local_34;
  int local_30;
  undefined4 local_2c;
  undefined4 local_28;
  int *local_24;
  undefined1 local_20 [12];
  float local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094e188;
  local_10 = ExceptionList;
  uVar3 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_8c = 0;
  local_88 = 0;
  local_84 = 0;
  local_2c = 0x7f000000;
  local_14 = _DAT_00993900;
  local_38 = 1;
  if ((float)param_3[2 = byte +0x8] <= (float)param_2[2 = byte +0x8]) {
    local_28 = param_3[2 = byte +0x8];
    local_34 = param_2[2 = byte +0x8];
  }
  else {
    local_28 = param_2[2 = byte +0x8];
    local_34 = param_3[2 = byte +0x8];
  }
  FUN_0084a640(0);
  local_8 = 0;
  local_24 = (int *)FUN_0051c0a0(param_6);
  if ((local_24 != (int *)0x0) && (iVar4 = (**(code **)(*local_24 + 0x10))(uVar3), iVar4 != 0)) {
    FUN_0083ea60(local_24);
  }
  local_24 = (int *)FUN_0051c0a0(param_7);
  if ((local_24 != (int *)0x0) && (iVar4 = (**(code **)(*local_24 + 0x10))(), iVar4 != 0)) {
    FUN_0083ea60(local_24);
  }
  FUN_0052be60();
  local_30 = FUN_0052d130(param_2,param_3,local_28,local_34,_DAT_0098bd74,0x7f000000,param_5,1,
                          local_20,param_8,local_80,0);
  if (local_30 == 0) {
    *param_4 = local_7c;
    param_4[1 = byte +0x4] = local_78;
    param_4[2 = byte +0x8] = local_74;
    local_38 = 0;
    if (param_9 == 0) goto LAB_0052be30;
  }
  for (local_90 = 0; local_90 < *(int *)(param_1 + 0x250); local_90 = local_90 + 1) {
    iVar4 = FUN_005ff950(*param_2,param_2[1 = byte +0x4],param_2[2 = byte +0x8],*param_3,param_3[1 = byte +0x4],param_3[2 = byte +0x8],&local_8c,
                         &local_2c);
    if (iVar4 == 0) {
      FUN_004452b0(&local_9c,&local_8c,param_2);
      fVar1 = local_9c * local_9c + local_98 * local_98 + local_94 * local_94;
/* ... 16 more lines truncated by --max-lines */
```

### decoy: 0x0052b9c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0052b9c0: 16-byte aligned: YES
  0052b9c0  55             PUSH EBP
  0052b9c1  8B EC          MOV EBP,ESP
  0052b9c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0052b9c0
string literals (2):
  0x009938F4  at 0052ba32  'BlockSight'
  0x009938F4  at 0052ba90  'BlockSight'
direct callees (6):
  0x0051C1A0  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x0083EA60  CExoArrayList<uint32>::Add
  0x0052D130  CSWSArea::NoNonWalkPolysDetailed
struct offsets touched (top 7):
  +0x130 [dec 304]  x2  via ECX
  +0x2E0 [dec 736]  x2  via EDX
  +0x8 [dec 8]  x1  via ECX
  +0x10 [dec 16]  x1  via EAX
  +0x28 [dec 40]  x1  via EAX
  +0x2C [dec 44]  x1  via EAX
  +0x2E1 [dec 737]  x1  via EDX
```

## Card B0-8-44

GOG function: CGameEffect::SetInteger  gog 0x00734b40  callconv=(blank)  param_size_bytes=8  gog_size=28  gog_retn=8
Candidate Steam address: 0x005e4e80 (size 28, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005e4e60:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00734b20 CGameEffect::GetInteger -> 0x005e4e60  gate_class=REVIEW
- GOG-order neighbour after:  0x00734b60 CGameEffect::GetFloat -> 0x005e4ea0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x34*} | Steam {0x34*}
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
prologue for 0x005e4e80: 16-byte aligned: YES
  005e4e80  55             PUSH EBP
  005e4e81  8B EC          MOV EBP,ESP
  005e4e83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005e4e80
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x34 [dec 52]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005e4e80 FUN_005e4e80 size=28 ret=8 callers=166 callees=0

void __thiscall FUN_005e4e80(int param_1,int param_2,undefined4 param_3)

{
  *(undefined4 *)(*(int *)(param_1 + 0x34) + param_2 * 4) = param_3;
  return;
}
```

### decoy: 0x005e4e60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005e4e60: 16-byte aligned: YES
  005e4e60  55             PUSH EBP
  005e4e61  8B EC          MOV EBP,ESP
  005e4e63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005e4e60
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x34 [dec 52]  x1  via EAX
```

## Card B0-8-45

GOG function: CSWSEffectListHandler::OnApplyTemporaryHitpoints  gog 0x0073d540  callconv=(blank)  param_size_bytes=8  gog_size=101  gog_retn=12
Candidate Steam address: 0x0059ac50 (size 101, retn 12)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0059abb0:STR0/CALLEE0/SIG0.208/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0073d4f0 CSWSEffectListHandler::OnRemoveDamageDecrease -> 0x0059aa40  gate_class=REVIEW
- GOG-order neighbour after:  0x0073d5b0 CSWSEffectListHandler::OnRemoveTemporaryHitpoints -> 0x0059acc0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x30*,0xe8*} | Steam {0x30*,0xe8*}
imm sets (* = in both, 2 common): GOG {0xc*,0x10*} | Steam {0xc*,0x10*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x0059ac50: 16-byte aligned: YES
  0059ac50  55             PUSH EBP
  0059ac51  8B EC          MOV EBP,ESP
  0059ac53  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059ac50
string literals (0):
direct callees (1):
  0x005E4E60  CGameEffect::GetInteger
struct offsets touched (top 2):
  +0x30 [dec 48]  x2  via EDX
  +0xE8 [dec 232]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0059ac50 FUN_0059ac50 size=101 ret=12 callers=0 callees=1

undefined4 __thiscall FUN_0059ac50(undefined4 param_1,int *param_2,int param_3)

{
  short sVar1;
  int iVar2;
  int iVar3;
  
  iVar2 = (**(code **)(*param_2 + 0x30))(param_1);
  if (iVar2 != 0) {
    iVar2 = (**(code **)(*param_2 + 0x30))();
    if (param_3 != 0) {
      sVar1 = *(short *)(iVar2 + 0xe8);
      iVar3 = FUN_005e4e60(0);
      *(int *)(iVar2 + 0xe8) = sVar1 + iVar3;
    }
  }
  return 0;
}
```

### decoy: 0x0059abb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059abb0: 16-byte aligned: YES
  0059abb0  55             PUSH EBP
  0059abb1  8B EC          MOV EBP,ESP
  0059abb3  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059abb0
string literals (0):
direct callees (2):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006BC570  CSWSCreatureStats::GetEffectImmunity
struct offsets touched (top 5):
  +0x30 [dec 48]  x3  via EDX
  +0x8 [dec 8]  x1  via EAX
  +0x18 [dec 24]  x1  via ECX
  +0x384 [dec 900]  x1  via ECX
  +0x1198 [dec 4504]  x1  via ECX
```

## Card B0-8-46

GOG function: CSWSArea::SetPlayerRestrictMode  gog 0x00723250  callconv=(blank)  param_size_bytes=4  gog_size=76  gog_retn=4
Candidate Steam address: 0x0052e170 (size 76, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch5_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0052e060:STR0/CALLEE0/SIG0.325/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007231f0 CSWSArea::SetTransitionPending -> 0x0052e110  gate_class=REVIEW
- GOG-order neighbour after:  0x007232a0 CSWSArea::GetNumMinesSetByPlayerParty -> 0x0052e1c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0064c2f0 CTwoDimArrays::GetSwoopUpgradeRowProvider -> Steam ?  in candidate callees: n/a
  0x007048d0 CSWPartyTable::UnstealthParty -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0x2e4*} | Steam {0x8*,0x2e4*}
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
prologue for 0x0052e170: 16-byte aligned: YES
  0052e170  55             PUSH EBP
  0052e171  8B EC          MOV EBP,ESP
  0052e173  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0052e170
string literals (0):
direct callees (2):
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005FEF20  CSWPartyTable::UnstealthParty
struct offsets touched (top 2):
  +0x2E4 [dec 740]  x2  via EAX,ECX
  +0x8 [dec 8]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0052e170 FUN_0052e170 size=76 ret=4 callers=3 callees=2

void __thiscall FUN_0052e170(int param_1,int param_2)

{
  undefined4 uVar1;
  
  if ((param_2 != 0) && (*(int *)(param_1 + 0x2e4) != param_2)) {
    uVar1 = 1;
    FUN_0051c8d0(1,param_1,*(undefined4 *)(DAT_00a1b4a4 + 8));
    FUN_005fef20(uVar1);
  }
  *(int *)(param_1 + 0x2e4) = param_2;
  return;
}
```

### decoy: 0x0052e060 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0052e060: 16-byte aligned: YES
  0052e060  55             PUSH EBP
  0052e061  8B EC          MOV EBP,ESP
  0052e063  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0052e060
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via ECX
  +0x8C [dec 140]  x1  via ECX
```

## Card B0-8-47

GOG function: CSWSEffectListHandler::OnApplyDamageImmunityIncrease  gog 0x0073da20  callconv=(blank)  param_size_bytes=8  gog_size=110  gog_retn=12
Candidate Steam address: 0x0059b130 (size 110, retn 12)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0059b1a0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0073d5b0 CSWSEffectListHandler::OnRemoveTemporaryHitpoints -> 0x0059acc0  gate_class=REVIEW
- GOG-order neighbour after:  0x0073da90 CSWSEffectListHandler::OnRemoveDamageImmunityIncrease -> 0x0059b1a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006a9ed0 CSWSObject::SetDamageImmunity -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xb4*} | Steam {0xb4*}
imm sets (* = in both, 1 common): GOG {0xc*} | Steam {0xc*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x0059b130: 16-byte aligned: YES
  0059b130  55             PUSH EBP
  0059b131  8B EC          MOV EBP,ESP
  0059b133  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059b130
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x005431F0  (unnamed)
struct offsets touched (top 1):
  +0xB4 [dec 180]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0059b130 FUN_0059b130 size=110 ret=12 callers=0 callees=2

undefined4 FUN_0059b130(int *param_1,int param_2)

{
  char cVar1;
  undefined2 uVar2;
  int iVar3;
  undefined4 extraout_var;
  undefined4 uVar4;
  
  if (param_2 != 0) {
    uVar2 = FUN_005e4e60(0);
    uVar4 = extraout_var;
    iVar3 = FUN_005e4e60(1);
    if (iVar3 < 0) {
      return 1;
    }
    cVar1 = (**(code **)(*param_1 + 0xb4))(uVar2,uVar4);
    FUN_005431f0(uVar2,cVar1 + iVar3 & 0xff);
  }
  return 0;
}
```

### decoy: 0x0059b1a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059b1a0: 16-byte aligned: YES
  0059b1a0  55             PUSH EBP
  0059b1a1  8B EC          MOV EBP,ESP
  0059b1a3  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059b1a0
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x005431F0  (unnamed)
struct offsets touched (top 3):
  +0x8 [dec 8]  x3  via EDX
  +0x4 [dec 4]  x1  via ECX
  +0xB4 [dec 180]  x1  via EDX
```

## Card B0-8-48

GOG function: CSWSEffectListHandler::OnApplyForceShield  gog 0x007370f0  callconv=(blank)  param_size_bytes=8  gog_size=1841  gog_retn=12
Candidate Steam address: 0x00594810 (size 1841, retn 12)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 13 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,SIG  (content level C2)
- evidence summary: STR 'Permanent'@00996260; 'Resistance'@00996274; 'VulnerFlags'@00996280; SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00594740:STR0/CALLEE0/SIG0.166/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00737080 CSWSEffectListHandler::OnEffectRemoved -> 0x005947a0  gate_class=REVIEW
- GOG-order neighbour after:  0x00737830 CSWSEffectListHandler::OnApplyFury -> 0x00594f50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (14), mapped to Steam through trusted rows only:
  0x0042ab80 CSWCCreature::GetForceShield -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605810 CExoString::Constructor_5 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x006059e0 CExoString::operator=_2 -> Steam ?  in candidate callees: n/a
  0x00621570 C2DA::GetINTEntry -> Steam ?  in candidate callees: n/a
  0x006a3970 CSWSCreature::GetClientCreature -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  ... 6 more GOG callees
GOG strings (14): 'DamageFlags'; 'Permanent'; 'Amount'; 'Resistance' ...
disp sets (* = in both, 7 common): GOG {0x4*,0x8*,0xa*,0x30*,0x110*,0x130*,0x1184*} | Steam {0x4*,0x8*,0xa*,0x30*,0x110*,0x130*,0x1184*}
imm sets (* = in both, 21 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0x1e*,0x6b*,0x8c*,...(+7)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0x1e*,0x6b*,0x8c*,...(+7)}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x00594810: 16-byte aligned: YES
  00594810  55             PUSH EBP
  00594811  8B EC          MOV EBP,ESP
  00594813  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00594810
string literals (14):
  0x00996318  at 005949ad  'VisualEffectDef'
  0x00996308  at 005949be  'Appearance_01'
  0x009962F8  at 00594a47  'VisualEffect_01'
  0x009962E8  at 00594a59  'Appearance_02'
  0x009962D8  at 00594ae2  'VisualEffect_02'
  0x009962C8  at 00594af4  'Appearance_03'
  0x009962B8  at 00594b7d  'VisualEffect_03'
  0x009962A8  at 00594b8f  'Appearance_04'
  0x00996298  at 00594c27  'VisualEffect_04'
  0x0099628C  at 00594c6b  'DamageFlags'
  0x00996280  at 00594ca5  'VulnerFlags'
  0x00996274  at 00594cdf  'Resistance'
direct callees (14):
  0x0058AE90  Creature::GetServerObjectVt2c
  0x0077EB60  CSWCCreature::GetForceShield
  0x00544790  CSWSObject::RemoveEffectById
  0x005E4E60  CGameEffect::GetInteger
  0x00733700  CExoString::Constructor_5
  0x00733570  CExoString::CExoString_char
  0x0071BAD0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x007338D0  CExoString::operator=_2
  0x00919723  operator new
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 7):
  +0x4 [dec 4]  x4  via EAX,EDX
  +0x8 [dec 8]  x3  via EAX,ECX
  +0xA [dec 10]  x2  via EAX,ECX
  +0x30 [dec 48]  x2  via EDX
  +0x110 [dec 272]  x1  via ECX
  +0x130 [dec 304]  x1  via EDX
  +0x1184 [dec 4484]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00594810 FUN_00594810 size=1841 ret=12 callers=0 callees=14

undefined4 FUN_00594810(int *param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  int local_104;
  int local_100;
  char local_fc;
  char local_f8;
  char local_f4;
  char local_f0;
  undefined1 local_b8 [8];
  undefined1 local_b0 [8];
  undefined1 local_a8 [8];
  undefined1 local_a0 [8];
  undefined1 local_98 [8];
  undefined1 local_90 [11];
  char local_85;
  undefined1 local_84 [11];
  char local_79;
  undefined1 local_78 [11];
  char local_6d;
  undefined1 local_6c [11];
  char local_61;
  ushort local_60;
  int local_5c;
  undefined4 *local_58;
  int *local_54;
  undefined4 local_50;
  undefined4 local_4c;
  undefined1 local_48 [8];
  undefined4 local_40;
  int local_3c;
  uint local_38;
  undefined4 local_34;
  int local_30;
  undefined4 local_2c;
  undefined4 local_28;
  ushort local_24;
  undefined1 local_20 [8];
  undefined4 local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00951d12;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (((param_1 != (int *)0x0) &&
      (iVar1 = (**(code **)(*param_1 + 0x30))(DAT_00a11f20 ^ (uint)&stack0xfffffffc), iVar1 != 0))
     && (param_2 != 0)) {
    local_30 = (**(code **)(*param_1 + 0x30))();
    local_24 = *(ushort *)(local_30 + 0x1184);
    iVar1 = FUN_0058ae90();
    if (iVar1 != 0) {
      FUN_0058ae90();
      iVar1 = FUN_0077eb60();
      if (iVar1 != 0) {
        local_58 = (undefined4 *)0x0;
        local_54 = (int *)(local_30 + 0x148);
        for (local_5c = 0; local_5c < *(int *)(local_30 + 0x14c); local_5c = local_5c + 1) {
          local_60 = *(ushort *)(*(int *)(*local_54 + local_5c * 4) + 8);
          if (local_60 == 0x6b) {
            local_58 = *(undefined4 **)(*local_54 + local_5c * 4);
            break;
          }
          if (0x6b < local_60) break;
        }
        FUN_00544790(*local_58,local_58[1 = byte +0x4]);
      }
    }
    uVar2 = FUN_005e4e60(0);
    FUN_00733700(uVar2);
    local_8 = 0;
    local_50 = *(undefined4 *)(*(int *)(DAT_00a1b4d0 + 0x130) + 0x110);
    local_38 = 0;
    local_2c = 0;
/* ... 140 more lines truncated by --max-lines */
```

### decoy: 0x00594740 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00594740: 16-byte aligned: YES
  00594740  55             PUSH EBP
  00594741  8B EC          MOV EBP,ESP
  00594743  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00594740
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x2  via EAX
  +0x8 [dec 8]  x1  via EAX
```

## Card B0-8-49

GOG function: CSWSEffectListHandler::OnApplyKnockdown  gog 0x0073e1c0  callconv=(blank)  param_size_bytes=12  gog_size=1441  gog_retn=12
Candidate Steam address: 0x0059b8d0 (size 1441, retn 12)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0059b520:STR0/CALLEE0/SIG0.333/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0073e160 CSWSEffectListHandler::OnRemoveEntangled -> 0x0059b870  gate_class=REVIEW
- GOG-order neighbour after:  0x0073e770 CSWSEffectListHandler::OnRemoveKnockdown -> 0x0059be80  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (15), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: yes
  0x00549c20 CSWCCMessageData::Constructor -> Steam ?  in candidate callees: n/a
  0x00549d60 CSWCCMessageData::SetObjectID -> Steam ?  in candidate callees: n/a
  0x0054a2c0 CSWCCMessageData::CopyTo -> Steam ?  in candidate callees: n/a
  0x0064baa0 CServerExoApp::GetObjectArray -> Steam ?  in candidate callees: n/a
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x0069d880 CSWSCreature::SendFeedbackMessage -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  ... 7 more GOG callees
GOG strings (0): 
disp sets (* = in both, 15 common): GOG {0x4*,0x8*,0xa*,0xc*,0x18*,0x30*,0x7c*,0x94*,0x9c*,0xec*,0xfc*,0xff5*,0x10dc*,0x1198*,...(+1)} | Steam {0x4*,0x8*,0xa*,0xc*,0x18*,0x30*,0x7c*,0x94*,0x9c*,0xec*,0xfc*,0xff5*,0x10dc*,0x1198*,...(+1)}
imm sets (* = in both, 18 common): GOG {0x2*,0x3*,0x4*,0x8*,0xc*,0xe*,0x17*,0x1c*,0x34*,0x81*,0x8c*,0x94*,0xa0*,0xe0*,...(+4)} | Steam {0x2*,0x3*,0x4*,0x8*,0xc*,0xe*,0x17*,0x1c*,0x34*,0x81*,0x8c*,0x94*,0xa0*,0xe0*,...(+4)}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x0059b8d0: 16-byte aligned: YES
  0059b8d0  55             PUSH EBP
  0059b8d1  8B EC          MOV EBP,ESP
  0059b8d3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0059b8d0
string literals (0):
direct callees (15):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006BC570  CSWSCreatureStats::GetEffectImmunity
  0x00919723  operator new
  0x005E2540  (unnamed)
  0x005E1C70  (unnamed)
  0x005E2BE0  (unnamed)
  0x00584DA0  CSWSCreature::SendFeedbackMessage
  0x0058FC60  CSWSCombatRound::SetRoundPaused
  0x0058FCD0  CSWSCombatRound::SetPauseTimer
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
  0x0051C080  (unnamed)
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005143C0  (unnamed)
struct offsets touched (top 10):
  +0x8 [dec 8]  x8  via EAX,ECX,EDX
  +0x4 [dec 4]  x5  via EAX,ECX,EDX
  +0x30 [dec 48]  x4  via EAX,EDX
  +0x7C [dec 124]  x3  via EDX
  +0xA [dec 10]  x2  via ECX
  +0xC [dec 12]  x2  via ECX,EDX
  +0x10DC [dec 4316]  x2  via EDX
  +0x18 [dec 24]  x1  via EAX
  +0x94 [dec 148]  x1  via EAX
  +0x9C [dec 156]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0059b8d0 FUN_0059b8d0 size=1441 ret=12 callers=0 callees=15

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_0059b8d0(int *param_1,int param_2,undefined4 param_3)

{
  bool bVar1;
  char cVar2;
  short sVar3;
  int iVar4;
  float *pfVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  int **ppiVar8;
  int local_f0;
  int local_ec;
  undefined4 local_e8;
  undefined4 local_e4;
  undefined1 local_88 [12];
  undefined1 local_7c [12];
  float local_70;
  float local_6c;
  float local_68;
  int local_64;
  int local_60;
  int local_5c;
  undefined4 local_58;
  int local_54;
  undefined4 local_50;
  int local_4c;
  undefined4 local_48;
  undefined4 local_44;
  int local_40;
  int local_3c;
  double local_38;
  float local_2c;
  float local_28;
  float local_24;
  int *local_20;
  int *local_1c;
  float local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009520bf;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_38 = _DAT_00996378;
  local_2c = 0.0;
  local_28 = 0.0;
  local_24 = 0.0;
  local_14 = *(undefined4 *)(param_2 + 0x18);
  local_20 = (int *)(**(code **)(*param_1 + 0x30))(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  if (local_20 != (int *)0x0) {
    local_40 = FUN_0051c100(local_14);
    (**(code **)(*param_1 + 0x30))();
    iVar4 = FUN_006bc570(0x1c,local_40);
    if (iVar4 != 0) {
      local_54 = FUN_00919723(0x34);
      local_8 = 0;
      if (local_54 == 0) {
        local_e4 = 0;
      }
      else {
        local_e4 = FUN_005e2540();
      }
      local_50 = local_e4;
      local_8 = 0xffffffff;
      local_48 = local_e4;
      FUN_005e1c70(0,param_1[1 = byte +0x4]);
      if (local_40 != 0) {
        local_5c = FUN_00919723(0x34);
        local_8 = 1;
        if (local_5c == 0) {
          local_e8 = 0;
        }
        else {
/* ... 95 more lines truncated by --max-lines */
```

### decoy: 0x0059b520 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059b520: 16-byte aligned: YES
  0059b520  55             PUSH EBP
  0059b521  8B EC          MOV EBP,ESP
  0059b523  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0059b520
string literals (0):
direct callees (11):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006BC570  CSWSCreatureStats::GetEffectImmunity
  0x00919723  operator new
  0x005E2540  (unnamed)
  0x005E1C70  (unnamed)
  0x005E2BE0  (unnamed)
  0x00584DA0  CSWSCreature::SendFeedbackMessage
  0x00541080  CSWSObject::ClearAllActions
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 7):
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0x30 [dec 48]  x3  via EDX
  +0x4 [dec 4]  x1  via EDX
  +0x18 [dec 24]  x1  via ECX
  +0xFC [dec 252]  x1  via ECX
  +0x122 [dec 290]  x1  via EDX
  +0x1198 [dec 4504]  x1  via EAX
```

## Card B0-8-50

GOG function: CSWSEffectListHandler::OnRemoveEntangled  gog 0x0073e160  callconv=(blank)  param_size_bytes=4  gog_size=95  gog_retn=8
Candidate Steam address: 0x0059b870 (size 95, retn 8)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0059b520:STR0/CALLEE0/SIG0.098/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0073de10 CSWSEffectListHandler::OnApplyEntangled -> 0x0059b520  gate_class=REVIEW
- GOG-order neighbour after:  0x0073e1c0 CSWSEffectListHandler::OnApplyKnockdown -> 0x0059b8d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006dbb50 CSWSCreature::UpdateVisibleList -> Steam ?  in candidate callees: n/a
  0x006dc130 CSWSCreature::ClearVisibleList -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x30*,0x10e8*} | Steam {0x30*,0x10e8*}
imm sets (* = in both, 2 common): GOG {0x8*,0xc*} | Steam {0x8*,0xc*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 8)
```

### prologue
```
prologue for 0x0059b870: 16-byte aligned: YES
  0059b870  55             PUSH EBP
  0059b871  8B EC          MOV EBP,ESP
  0059b873  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059b870
string literals (0):
direct callees (2):
  0x005D1510  CSWSCreature::ClearVisibleList
  0x005D0F30  CSWSObject::UpdatePerception
struct offsets touched (top 2):
  +0x30 [dec 48]  x2  via EDX
  +0x10E8 [dec 4328]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0059b870 FUN_0059b870 size=95 ret=8 callers=0 callees=2

undefined4 __thiscall FUN_0059b870(undefined4 param_1,int *param_2)

{
  int iVar1;
  
  if (param_2 != (int *)0x0) {
    iVar1 = (**(code **)(*param_2 + 0x30))(param_1);
    if (iVar1 != 0) {
      iVar1 = (**(code **)(*param_2 + 0x30))();
      if (*(int *)(iVar1 + 0x10e8) == 0) {
        FUN_005d1510();
        FUN_005d0f30(0);
      }
    }
  }
  return 1;
}
```

### decoy: 0x0059b520 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059b520: 16-byte aligned: YES
  0059b520  55             PUSH EBP
  0059b521  8B EC          MOV EBP,ESP
  0059b523  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0059b520
string literals (0):
direct callees (11):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006BC570  CSWSCreatureStats::GetEffectImmunity
  0x00919723  operator new
  0x005E2540  (unnamed)
  0x005E1C70  (unnamed)
  0x005E2BE0  (unnamed)
  0x00584DA0  CSWSCreature::SendFeedbackMessage
  0x00541080  CSWSObject::ClearAllActions
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 7):
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0x30 [dec 48]  x3  via EDX
  +0x4 [dec 4]  x1  via EDX
  +0x18 [dec 24]  x1  via ECX
  +0xFC [dec 252]  x1  via ECX
  +0x122 [dec 290]  x1  via EDX
  +0x1198 [dec 4504]  x1  via EAX
```

## Card B0-8-51

GOG function: CSWSEffectListHandler::OnApplyEntangled  gog 0x0073de10  callconv=(blank)  param_size_bytes=12  gog_size=836  gog_retn=12
Candidate Steam address: 0x0059b520 (size 836, retn 12)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0059b2e0:STR0/CALLEE0/SIG0.346/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0073dcd0 CSWSEffectListHandler::OnRemoveDamageImmunityDecrease -> 0x0059b3e0  gate_class=REVIEW
- GOG-order neighbour after:  0x0073e160 CSWSEffectListHandler::OnRemoveEntangled -> 0x0059b870  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (11), mapped to Steam through trusted rows only:
  0x00549c20 CSWCCMessageData::Constructor -> Steam ?  in candidate callees: n/a
  0x00549d60 CSWCCMessageData::SetObjectID -> Steam ?  in candidate callees: n/a
  0x0054a2c0 CSWCCMessageData::CopyTo -> Steam ?  in candidate callees: n/a
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x0069d880 CSWSCreature::SendFeedbackMessage -> Steam ?  in candidate callees: n/a
  0x006a7d60 CSWSObject::ClearAllActions -> Steam 0x00541080  in candidate callees: yes
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x006f89f0 CSWSCreatureStats::GetEffectImmunity -> Steam ?  in candidate callees: n/a
  ... 3 more GOG callees
GOG strings (0): 
disp sets (* = in both, 7 common): GOG {0x4*,0x8*,0x18*,0x30*,0xfc*,0x122*,0x1198*} | Steam {0x4*,0x8*,0x18*,0x30*,0xfc*,0x122*,0x1198*}
imm sets (* = in both, 13 common): GOG {0x2*,0x3*,0x4*,0xa*,0xb*,0xc*,0x17*,0x25*,0x34*,0x74*,0x8c*,0x90*,0xfffffffd*} | Steam {0x2*,0x3*,0x4*,0xa*,0xb*,0xc*,0x17*,0x25*,0x34*,0x74*,0x8c*,0x90*,0xfffffffd*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x0059b520: 16-byte aligned: YES
  0059b520  55             PUSH EBP
  0059b521  8B EC          MOV EBP,ESP
  0059b523  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0059b520
string literals (0):
direct callees (11):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006BC570  CSWSCreatureStats::GetEffectImmunity
  0x00919723  operator new
  0x005E2540  (unnamed)
  0x005E1C70  (unnamed)
  0x005E2BE0  (unnamed)
  0x00584DA0  CSWSCreature::SendFeedbackMessage
  0x00541080  CSWSObject::ClearAllActions
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 7):
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0x30 [dec 48]  x3  via EDX
  +0x4 [dec 4]  x1  via EDX
  +0x18 [dec 24]  x1  via ECX
  +0xFC [dec 252]  x1  via ECX
  +0x122 [dec 290]  x1  via EDX
  +0x1198 [dec 4504]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0059b520 FUN_0059b520 size=836 ret=12 callers=0 callees=11

undefined4 FUN_0059b520(int *param_1,int param_2,undefined4 param_3)

{
  int iVar1;
  int iVar2;
  int iVar3;
  undefined4 uVar4;
  int local_84;
  int local_80;
  int local_7c;
  undefined4 local_78;
  undefined4 local_74;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952067;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = (**(code **)(*param_1 + 0x30))(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  if (iVar1 != 0) {
    iVar1 = (**(code **)(*param_1 + 0x30))();
    iVar2 = FUN_0051c100(*(undefined4 *)(param_2 + 0x18));
    iVar3 = FUN_006bc570(10,iVar2);
    if (iVar3 != 0) {
      iVar1 = FUN_00919723(0x34);
      local_8 = 0;
      if (iVar1 == 0) {
        local_74 = 0;
      }
      else {
        local_74 = FUN_005e2540();
      }
      local_8 = 0xffffffff;
      FUN_005e1c70(0,param_1[1 = byte +0x4]);
      if (iVar2 != 0) {
        iVar1 = FUN_00919723(0x34);
        local_8 = 1;
        if (iVar1 == 0) {
          local_78 = 0;
        }
        else {
          local_78 = FUN_005e2540();
        }
        local_8 = 0xffffffff;
        FUN_005e2be0(local_78);
        FUN_00584da0(0x90,local_78);
      }
      uVar4 = 0x90;
      (**(code **)(*param_1 + 0x30))(0x90,local_74);
      FUN_00584da0(uVar4,local_74);
      ExceptionList = local_10;
      return 1;
    }
    if (*(int *)(iVar1 + 0xfc) != 0) {
      ExceptionList = local_10;
      return 1;
    }
    if (param_2 != 0) {
      FUN_00541080(1);
      iVar1 = FUN_00919723(0x8c);
      local_8 = 2;
      if (iVar1 == 0) {
        local_7c = 0;
      }
      else {
        local_7c = FUN_005e4ac0(param_2);
      }
      local_8 = 0xffffffff;
      iVar1 = FUN_00919723(0x8c);
      local_8 = 3;
      if (iVar1 == 0) {
        local_80 = 0;
      }
      else {
        local_80 = FUN_005e4ac0(param_2);
      }
      local_8 = 0xffffffff;
/* ... 26 more lines truncated by --max-lines */
```

### decoy: 0x0059b2e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059b2e0: 16-byte aligned: YES
  0059b2e0  55             PUSH EBP
  0059b2e1  8B EC          MOV EBP,ESP
  0059b2e3  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059b2e0
string literals (0):
direct callees (4):
  0x005E4E60  CGameEffect::GetInteger
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006BC570  CSWSCreatureStats::GetEffectImmunity
  0x005431F0  (unnamed)
struct offsets touched (top 6):
  +0x30 [dec 48]  x2  via EAX
  +0x8 [dec 8]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
  +0xB4 [dec 180]  x1  via EDX
  +0xFC [dec 252]  x1  via EAX
  +0x1198 [dec 4504]  x1  via EAX
```

## Card B0-8-52

GOG function: CGameEffect::SetString  gog 0x00734c20  callconv=(blank)  param_size_bytes=8  gog_size=32  gog_retn=8
Candidate Steam address: 0x005e4f60 (size 32, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005e4f20:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00734be0 CGameEffect::GetString -> 0x005e4f20  gate_class=REVIEW
- GOG-order neighbour after:  0x00734c40 CGameEffect::SetExpiryTime -> 0x005e4f80  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x48*} | Steam {0x48*}
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
prologue for 0x005e4f60: 16-byte aligned: YES
  005e4f60  55             PUSH EBP
  005e4f61  8B EC          MOV EBP,ESP
  005e4f63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005e4f60
string literals (0):
direct callees (1):
  0x007337C0  CExoString::operator=
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005e4f60 FUN_005e4f60 size=32 ret=8 callers=3 callees=1
THUNK -> 0x007337C0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_005e4f60(undefined4 param_1,undefined4 param_2)

{
  FUN_007337c0(param_2);
  return;
}
```

### decoy: 0x005e4f20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005e4f20: 16-byte aligned: YES
  005e4f20  55             PUSH EBP
  005e4f21  8B EC          MOV EBP,ESP
  005e4f23  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005e4f20
string literals (0):
direct callees (1):
  0x007335F0  CExoString::CExoString_copy
struct offsets touched (top 0):
```

## Card B0-8-53

GOG function: CSWSItemPropertyHandler::ApplyACBonus  gog 0x007c53c0  callconv=__thiscall  param_size_bytes=12  gog_size=686  gog_retn=20
Candidate Steam address: 0x005ae530 (size 335, retn 20)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSItemPropertyHandler::ApplyImprovedForceResistance
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.45 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005ae260:STR0/CALLEE0/SIG0.488/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c5100 CSWSItemPropertyHandler::RemoveAbilityBonus -> 0x005adcf0  gate_class=REVIEW
- GOG-order neighbour after:  0x007c5670 CSWSItemPropertyHandler::RemoveACBonus -> 0x005ae260  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (12), mapped to Steam through trusted rows only:
  0x00489130 CTwoDimArrays::GetIPRPCostTable -> Steam ?  in candidate callees: n/a
  0x004eafd0 CSWBaseItemArray::GetBaseItem -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734960 CGameEffect::Destructor -> Steam ?  in candidate callees: n/a
  ... 4 more GOG callees
GOG strings (1): 'Value'
disp sets (* = in both, 5 common): GOG {0x2,0x6*,0x8*,0xa*,0xc,0x14*,0x34,0x8b,0x122,0x130*} | Steam {0x6*,0x8*,0xa*,0x14*,0x130*}
imm sets (* = in both, 4 common): GOG {0x2,0x3,0x4*,0x5,0x14*,0x64,0x8c*,0xfffffff8*} | Steam {0x4*,0xb,0x14*,0x34,0x8c*,0xfffffff8*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 20)
```

### prologue
```
prologue for 0x005ae530: 16-byte aligned: YES
  005ae530  55             PUSH EBP
  005ae531  8B EC          MOV EBP,ESP
  005ae533  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005ae530
string literals (1):
  0x00992E8C  at 005ae558  'Value'
direct callees (9):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 5):
  +0xA [dec 10]  x2  via ECX
  +0x6 [dec 6]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
  +0x14 [dec 20]  x1  via EDX
  +0x130 [dec 304]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005ae530 FUN_005ae530 size=335 ret=20 callers=0 callees=9

undefined4
FUN_005ae530(int param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  undefined1 *puVar4;
  int *piVar5;
  int local_44;
  undefined2 local_34;
  undefined1 local_20 [8];
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952e93;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570("Value");
  local_8 = 0;
  piVar5 = &local_14;
  puVar4 = local_20;
  uVar2 = (uint)*(ushort *)(param_2 + 6);
  FUN_006f0190(0xb);
  FUN_0071bcf0(uVar2,puVar4,piVar5);
  local_8 = 0xffffffff;
  FUN_00733780(uVar1);
  if (local_14 != 0) {
    iVar3 = FUN_00919723(0x8c);
    local_8 = 1;
    if (iVar3 == 0) {
      local_44 = 0;
    }
    else {
      local_44 = FUN_005e4920(1);
    }
    local_8 = 0xffffffff;
    local_18 = local_44;
    local_34 = (undefined2)DAT_00995fa0;
    *(undefined2 *)(local_44 + 8) = local_34;
    *(ushort *)(local_44 + 0xa [dec 10]) = *(ushort *)(local_44 + 0xa [dec 10]) & 0xfff8 | DAT_009967b0;
    FUN_005e5ea0(*(undefined4 *)(param_1 + 0x14));
    FUN_005e4e80(0,local_14);
    FUN_00544210(local_18,param_5,0);
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005ae260 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005ae260: 16-byte aligned: YES
  005ae260  55             PUSH EBP
  005ae261  8B EC          MOV EBP,ESP
  005ae263  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005ae260
string literals (1):
  0x00992E8C  at 005ae293  'Value'
direct callees (7):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x006D5200  (unnamed)
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 10):
  +0x2 [dec 2]  x4  via EAX,ECX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x6 [dec 6]  x1  via ECX
  +0xA [dec 10]  x1  via EAX
  +0xC [dec 12]  x1  via EDX
  +0x14 [dec 20]  x1  via EAX
  +0x18 [dec 24]  x1  via ECX
  +0x34 [dec 52]  x1  via EAX
  +0x8B [dec 139]  x1  via ECX
```

## Card B0-8-54

GOG function: CSWSEffectListHandler::OnEffectRemoved  gog 0x00737080  callconv=(blank)  param_size_bytes=8  gog_size=100  gog_retn=8
Candidate Steam address: 0x005947a0 (size 100, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00594740:STR0/CALLEE0/SIG0.583/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00737020 CSWSEffectListHandler::OnEffectApplied -> 0x00594740  gate_class=REVIEW
- GOG-order neighbour after:  0x007370f0 CSWSEffectListHandler::OnApplyForceShield -> 0x00594810  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 3 common): GOG {0x8*,0xc*,0x76*} | Steam {0x8*,0xc*,0x76*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x005947a0: 16-byte aligned: YES
  005947a0  55             PUSH EBP
  005947a1  8B EC          MOV EBP,ESP
  005947a3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005947a0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x8 [dec 8]  x4  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005947a0 FUN_005947a0 size=100 ret=8 callers=0 callees=0

undefined4 __thiscall FUN_005947a0(int param_1,undefined4 param_2,int param_3)

{
  undefined4 uVar1;
  
  if ((*(ushort *)(param_3 + 8) < 0x76) &&
     (*(int *)(*(int *)(param_1 + 8) + (uint)*(ushort *)(param_3 + 8) * 4) != 0)) {
    uVar1 = (**(code **)(*(int *)(param_1 + 8) + (uint)*(ushort *)(param_3 + 8) * 4))
                      (param_2,param_3);
  }
  else {
    uVar1 = 1;
  }
  return uVar1;
}
```

### decoy: 0x00594740 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00594740: 16-byte aligned: YES
  00594740  55             PUSH EBP
  00594741  8B EC          MOV EBP,ESP
  00594743  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00594740
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x2  via EAX
  +0x8 [dec 8]  x1  via EAX
```

## Card B0-8-55

GOG function: CGameEffect::Constructor  gog 0x007345e0  callconv=(blank)  param_size_bytes=4  gog_size=416  gog_retn=4
Candidate Steam address: 0x005e4920 (size 416, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005e4ac0:STR0/CALLEE0/SIG0.971/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00734550 CSWSRoom::GetNormalOfPoint  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00734780 CGameEffect::Constructor_2 -> 0x005e4ac0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x006059e0 CExoString::operator=_2 -> Steam ?  in candidate callees: n/a
  0x00734a90 CGameEffect::SetNumIntegers -> Steam ?  in candidate callees: n/a
  0x00924260 Global::memset -> Steam ?  in candidate callees: n/a
  0x009258bf Global::`eh_vector_constructor_iterator' -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 16 common): GOG {0x4*,0x8*,0xa*,0xc*,0x10*,0x14*,0x18*,0x1c*,0x20*,0x24*,0x28*,0x2c*,0x34*,0x48*,...(+2)} | Steam {0x4*,0x8*,0xa*,0xc*,0x10*,0x14*,0x18*,0x1c*,0x20*,0x24*,0x28*,0x2c*,0x34*,0x48*,...(+2)}
imm sets (* = in both, 8 common): GOG {0x4*,0x6*,0x8*,0xc*,0x10*,0x38*,0x48*,0x7f000000*} | Steam {0x4*,0x6*,0x8*,0xc*,0x10*,0x38*,0x48*,0x7f000000*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005e4920: 16-byte aligned: YES
  005e4920  55             PUSH EBP
  005e4921  8B EC          MOV EBP,ESP
  005e4923  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005e4920
string literals (0):
direct callees (4):
  0x0091D67E  (unnamed)
  0x005E4DD0  CGameEffect::SetNumIntegers
  0x0091C630  (unnamed)
  0x007338D0  CExoString::operator=_2
struct offsets touched (top 10):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x1  via EDX
  +0xA [dec 10]  x1  via ECX
  +0xC [dec 12]  x1  via EDX
  +0x10 [dec 16]  x1  via EAX
  +0x14 [dec 20]  x1  via ECX
  +0x18 [dec 24]  x1  via EDX
  +0x1C [dec 28]  x1  via EAX
  +0x20 [dec 32]  x1  via ECX
  +0x24 [dec 36]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005e4920 FUN_005e4920 size=416 ret=4 callers=166 callees=4

uint * __thiscall FUN_005e4920(uint *param_1,uint param_2)

{
  bool bVar1;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00953dd6;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  _eh_vector_constructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable](param_1 + 0x12,8,6,FUN_00733540,FUN_00733780);
  local_8 = 0;
  if (param_2 == 1) {
    *param_1 = DAT_00a0f700;
    param_1[1 = byte +0x4] = DAT_00a0f704;
    bVar1 = 0xfffffffe < DAT_00a0f700;
    DAT_00a0f700 = DAT_00a0f700 + 1;
    DAT_00a0f704 = DAT_00a0f704 + bVar1;
  }
  else {
    *param_1 = 0;
    param_1[1 = byte +0x4] = 0;
  }
  param_1[8 = byte +0x20] = param_2;
  param_1[0x22 = byte +0x88] = 0;
  *(undefined2 *)(param_1 + 0x2 [dec 2] {uint*: byte +0x8}) = 0;
  *(undefined2 *)((int)param_1 + 0xa [dec 10]) = 0;
  param_1[3 = byte +0xc] = 0;
  param_1[4 = byte +0x10] = 0;
  param_1[5 = byte +0x14] = 0;
  param_1[6 = byte +0x18] = 0x7f000000;
  param_1[10 = byte +0x28] = 0;
  param_1[0xb = byte +0x2c] = 0;
  param_1[9 = byte +0x24] = 0xffffffff;
  param_1[7 = byte +0x1c] = 0xffffffff;
  param_1[0xd = byte +0x34] = 0;
  FUN_005e4dd0(8);
  _memset [GHIDRA-AUTO-LABEL: unreliable](param_1 + 0xe,0,0x10);
  for (local_14 = 0; local_14 < 6; local_14 = local_14 + 1) {
    FUN_007338d0(&DAT_00986925);
  }
  for (local_14 = 0; local_14 < 4; local_14 = local_14 + 1) {
    param_1[local_14 + 0x1e] = 0x7f000000;
  }
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x005e4ac0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005e4ac0: 16-byte aligned: YES
  005e4ac0  55             PUSH EBP
  005e4ac1  8B EC          MOV EBP,ESP
  005e4ac3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005e4ac0
string literals (0):
direct callees (4):
  0x0091D67E  (unnamed)
  0x005E4DD0  CGameEffect::SetNumIntegers
  0x0091C630  (unnamed)
  0x007338D0  CExoString::operator=_2
struct offsets touched (top 10):
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
  +0xA [dec 10]  x3  via EAX,ECX,EDX
  +0xC [dec 12]  x3  via ECX,EDX
  +0x10 [dec 16]  x3  via EAX,ECX,EDX
  +0x14 [dec 20]  x3  via EAX,ECX
  +0x18 [dec 24]  x3  via EAX,ECX
  +0x1C [dec 28]  x3  via EAX,ECX,EDX
  +0x8 [dec 8]  x1  via ECX
  +0x20 [dec 32]  x1  via EDX
  +0x24 [dec 36]  x1  via EAX
```

## Card B0-8-56

GOG function: CSWSArea::SetTransitionPending  gog 0x007231f0  callconv=(blank)  param_size_bytes=5  gog_size=94  gog_retn=8
Candidate Steam address: 0x0052e110 (size 94, retn 8)
Candidate sources: branch+name_map:high:batch5_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0052e290:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00723140 CSWSArea::SetFogColor -> 0x0052e060  gate_class=AUTO
- GOG-order neighbour after:  0x00723250 CSWSArea::SetPlayerRestrictMode -> 0x0052e170  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0040d320 CClientExoApp::SetInputActive -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x2f8*,0x2fd*} | Steam {0x4*,0x2f8*,0x2fd*}
imm sets (* = in both, 2 common): GOG {0x8*,0xc*} | Steam {0x8*,0xc*}

### abi
```
GOG param bytes: 5; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(5/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 5 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0052e110: 16-byte aligned: YES
  0052e110  55             PUSH EBP
  0052e111  8B EC          MOV EBP,ESP
  0052e113  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0052e110
string literals (0):
direct callees (1):
  0x0073FE40  CClientExoApp::SetInputActive
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x2F8 [dec 760]  x1  via EAX
  +0x2FD [dec 765]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0052e110 FUN_0052e110 size=94 ret=8 callers=3 callees=1

void __thiscall FUN_0052e110(int param_1,int param_2,undefined1 param_3)

{
  *(int *)(param_1 + 0x2f8) = param_2;
  *(undefined1 *)(param_1 + 0x2fd) = param_3;
  if (param_2 == 0) {
    FUN_0073fe40(1,0);
  }
  else {
    FUN_0073fe40(0,1);
  }
  return;
}
```

### decoy: 0x0052e290 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0052e290: 16-byte aligned: YES
  0052e290  55             PUSH EBP
  0052e291  8B EC          MOV EBP,ESP
  0052e293  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0052e290
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

## Card B0-8-57

GOG function: CGameEffect::SetFloat  gog 0x00734b80  callconv=(blank)  param_size_bytes=8  gog_size=26  gog_retn=8
Candidate Steam address: 0x005e4ec0 (size 26, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005e4ea0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00734b60 CGameEffect::GetFloat -> 0x005e4ea0  gate_class=REVIEW
- GOG-order neighbour after:  0x00734ba0 CGameEffect::GetObjectID -> 0x005e4ee0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x38*} | Steam {0x38*}
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
prologue for 0x005e4ec0: 16-byte aligned: YES
  005e4ec0  55             PUSH EBP
  005e4ec1  8B EC          MOV EBP,ESP
  005e4ec3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005e4ec0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005e4ec0 FUN_005e4ec0 size=26 ret=8 callers=14 callees=0

void __thiscall FUN_005e4ec0(int param_1,int param_2,undefined4 param_3)

{
  *(undefined4 *)(param_1 + 0x38 + param_2 * 4) = param_3;
  return;
}
```

### decoy: 0x005e4ea0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005e4ea0: 16-byte aligned: YES
  005e4ea0  55             PUSH EBP
  005e4ea1  8B EC          MOV EBP,ESP
  005e4ea3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005e4ea0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

## Card B0-8-58

GOG function: CSWSArea::ClearLineOfSight  gog 0x00720d10  callconv=(blank)  param_size_bytes=32  gog_size=176  gog_retn=None
Candidate Steam address: 0x0052bb60 (size 767, retn 32)
Candidate sources: branch+mapper:vote+name_map:high:batch5_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: n/a | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.35 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (AIActionPhysicalAttacks 0x6d7390,ExecuteCommandHasLineOfSight 0x69ac60)
- best decoy (neighbour/other candidate) scores: 0x0052be60:STR0/CALLEE1/SIG0.181/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00720b70 CSWSArea::HandleTransparentDoors -> 0x0052b9c0  gate_class=AUTO
- GOG-order neighbour after:  0x00720dc0 CSWSArea::Constructor_2 -> 0x0052bb60  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00567c10 CExoArrayList_int_::CExoArrayList -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x4,0x8*,0x10,0x250,0x254}
imm sets (* = in both, 3 common): GOG {0x41*,0xc0*,0x7f000000*} | Steam {0xc,0x20,0x41*,0x4c,0xc0*,0x7f000000*}

### abi
```
GOG param bytes: 32; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(32/4)*4 = 32 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=32 ; asm RET = 32
Stack Slot Match: TRUE (GOG 32 bytes -> x86 slots 32; ASM RET 32)
```

### prologue
```
prologue for 0x0052bb60: 16-byte aligned: YES
  0052bb60  55             PUSH EBP
  0052bb61  8B EC          MOV EBP,ESP
  0052bb63  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0052bb60
string literals (0):
direct callees (8):
  0x0084A640  (unnamed)
  0x0051C0A0  CServerExoApp::GetGameObject
  0x0083EA60  CExoArrayList<uint32>::Add
  0x0052BE60  CSWSArea::Constructor_2
  0x0052D130  CSWSArea::NoNonWalkPolysDetailed
  0x005FF950  (unnamed)
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0064DDB0  Free(ptr*) delete-and-zero helper
struct offsets touched (top 5):
  +0x8 [dec 8]  x14  via EAX,ECX,EDX
  +0x4 [dec 4]  x6  via EAX,ECX,EDX
  +0x10 [dec 16]  x2  via EAX,EDX
  +0x250 [dec 592]  x1  via ECX
  +0x254 [dec 596]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0052bb60 FUN_0052bb60 size=767 ret=32 callers=20 callees=8

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall
FUN_0052bb60(int param_1,undefined4 *param_2,undefined4 *param_3,undefined4 *param_4,
            undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8,int param_9)

{
  float fVar1;
  undefined4 uVar2;
  uint uVar3;
  int iVar4;
  float local_9c;
  float local_98;
  float local_94;
  int local_90;
  undefined4 local_8c;
  undefined4 local_88;
  undefined4 local_84;
  undefined1 local_80 [4];
  undefined4 local_7c;
  undefined4 local_78;
  undefined4 local_74;
  undefined4 local_38;
  undefined4 local_34;
  int local_30;
  undefined4 local_2c;
  undefined4 local_28;
  int *local_24;
  undefined1 local_20 [12];
  float local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094e188;
  local_10 = ExceptionList;
  uVar3 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_8c = 0;
  local_88 = 0;
  local_84 = 0;
  local_2c = 0x7f000000;
  local_14 = _DAT_00993900;
  local_38 = 1;
  if ((float)param_3[2 = byte +0x8] <= (float)param_2[2 = byte +0x8]) {
    local_28 = param_3[2 = byte +0x8];
    local_34 = param_2[2 = byte +0x8];
  }
  else {
    local_28 = param_2[2 = byte +0x8];
    local_34 = param_3[2 = byte +0x8];
  }
  FUN_0084a640(0);
  local_8 = 0;
  local_24 = (int *)FUN_0051c0a0(param_6);
  if ((local_24 != (int *)0x0) && (iVar4 = (**(code **)(*local_24 + 0x10))(uVar3), iVar4 != 0)) {
    FUN_0083ea60(local_24);
  }
  local_24 = (int *)FUN_0051c0a0(param_7);
  if ((local_24 != (int *)0x0) && (iVar4 = (**(code **)(*local_24 + 0x10))(), iVar4 != 0)) {
    FUN_0083ea60(local_24);
  }
  FUN_0052be60();
  local_30 = FUN_0052d130(param_2,param_3,local_28,local_34,_DAT_0098bd74,0x7f000000,param_5,1,
                          local_20,param_8,local_80,0);
  if (local_30 == 0) {
    *param_4 = local_7c;
    param_4[1 = byte +0x4] = local_78;
    param_4[2 = byte +0x8] = local_74;
    local_38 = 0;
    if (param_9 == 0) goto LAB_0052be30;
  }
  for (local_90 = 0; local_90 < *(int *)(param_1 + 0x250); local_90 = local_90 + 1) {
    iVar4 = FUN_005ff950(*param_2,param_2[1 = byte +0x4],param_2[2 = byte +0x8],*param_3,param_3[1 = byte +0x4],param_3[2 = byte +0x8],&local_8c,
                         &local_2c);
    if (iVar4 == 0) {
      FUN_004452b0(&local_9c,&local_8c,param_2);
      fVar1 = local_9c * local_9c + local_98 * local_98 + local_94 * local_94;
/* ... 16 more lines truncated by --max-lines */
```

### decoy: 0x0052be60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0052be60: 16-byte aligned: YES
  0052be60  55             PUSH EBP
  0052be61  8B EC          MOV EBP,ESP
  0052be63  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0052be60
string literals (0):
direct callees (0):
struct offsets touched (top 4):
  +0x4 [dec 4]  x5  via EAX,ECX,EDX
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0x40 [dec 64]  x1  via ECX
  +0x44 [dec 68]  x1  via EAX
```

## Card B0-8-59

GOG function: CSWSEffectListHandler::OnApplyAttackIncrease  gog 0x0073d2d0  callconv=(blank)  param_size_bytes=8  gog_size=95  gog_retn=12
Candidate Steam address: 0x0059a9e0 (size 95, retn 12)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0059ab70:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0073d020 CSWSEffectListHandler::OnRemoveACDecrease -> 0x0059a730  gate_class=REVIEW
- GOG-order neighbour after:  0x0073d330 CSWSEffectListHandler::OnApplyAttackDecrease -> 0x0059aa90  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x30*,0x384*} | Steam {0x30*,0x384*}
imm sets (* = in both, 1 common): GOG {0xc*} | Steam {0xc*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x0059a9e0: 16-byte aligned: YES
  0059a9e0  55             PUSH EBP
  0059a9e1  8B EC          MOV EBP,ESP
  0059a9e3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059a9e0
string literals (0):
direct callees (1):
  0x005E4E60  CGameEffect::GetInteger
struct offsets touched (top 2):
  +0x30 [dec 48]  x2  via EDX
  +0x384 [dec 900]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0059a9e0 FUN_0059a9e0 size=95 ret=12 callers=0 callees=1

undefined4 __thiscall FUN_0059a9e0(undefined4 param_1,int *param_2,int param_3)

{
  int iVar1;
  
  iVar1 = (**(code **)(*param_2 + 0x30))(param_1);
  if ((iVar1 != 0) && (param_3 != 0)) {
    iVar1 = FUN_005e4e60(0);
    if (iVar1 < 1) {
      return 1;
    }
    iVar1 = (**(code **)(*param_2 + 0x30))();
    *(undefined4 *)(iVar1 + 0x384 [dec 900]) = 1;
  }
  return 0;
}
```

### decoy: 0x0059ab70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059ab70: 16-byte aligned: YES
  0059ab70  55             PUSH EBP
  0059ab71  8B EC          MOV EBP,ESP
  0059ab73  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059ab70
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x30 [dec 48]  x2  via EDX
  +0x384 [dec 900]  x1  via ECX
```

## Card B0-8-60

GOG function: CSWSArea::AttachmentFromClientArea  gog 0x00722cf0  callconv=(blank)  param_size_bytes=4  gog_size=25  gog_retn=4
Candidate Steam address: 0x0052dc10 (size 25, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch5_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0052dc30:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00722c70 CSWSArea::GetClientArea -> 0x0052db90  gate_class=AUTO
- GOG-order neighbour after:  0x00722d10 CSWSArea::DetachFromClientArea -> 0x0052dc30  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x334*} | Steam {0x334*}
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
prologue for 0x0052dc10: 16-byte aligned: YES
  0052dc10  55             PUSH EBP
  0052dc11  8B EC          MOV EBP,ESP
  0052dc13  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0052dc10
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x334 [dec 820]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0052dc10 FUN_0052dc10 size=25 ret=4 callers=1 callees=0

void __thiscall FUN_0052dc10(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0x334) = param_2;
  return;
}
```

### decoy: 0x0052dc30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0052dc30: 16-byte aligned: YES
  0052dc30  55             PUSH EBP
  0052dc31  8B EC          MOV EBP,ESP
  0052dc33  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0052dc30
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x334 [dec 820]  x1  via EAX
```
