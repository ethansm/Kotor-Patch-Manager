# Review cards B17 chunk 6

Answer each card with one TSV row in `steam_db_batches/B17/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B17-6-01

GOG function: CSWGuiNameChargen::Destructor  gog 0x00604f00  callconv=(blank)  param_size_bytes=0  gog_size=317  gog_retn=0
Candidate Steam address: 0x00918420 (size 774, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.02 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00918030:STR0/CALLEE0/SIG0.16/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00604ed0 CSWGuiNameChargen::scalar  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00605040 CSWGuiNameChargen::OnPanelAdded  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x004ce500 CSWGuiControl::Destructor -> Steam 0x00418640  in candidate callees: no
  0x004ce630 CSWGuiButton::Destructor -> Steam 0x0041a3d0  in candidate callees: no
  0x005d7d00 CSWGuiEditbox::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
  0x008115a0 CSWNameGen::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x0090cce0 CSWGuiPanel::Destructor -> Steam 0x0040e860  in candidate callees: no
  0x00913030 CSWGuiBorder::Destructor_2 -> Steam 0x00414c60  in candidate callees: no
  0x00915660 CSWGuiText::Destructor_2 -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8,0x18,0x48,0x68,0x80,0x94,0xd8,0xf4,0x124,0x148,0x224,0x310,0xb24,...(+4)}
imm sets (* = in both, 1 common): GOG {0x2*,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0x10,0x60,0x70,0xd8,...(+7)} | Steam {0x2*,0xf,0x3c,0x98,0xfe,0xff,0x374,0x3ea}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00918420: 16-byte aligned: YES
  00918420  55             PUSH EBP
  00918421  8B EC          MOV EBP,ESP
  00918423  81 EC 98 00 00 00 SUB ESP,0x98
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00918420
string literals (2):
  0x0099F35C  at 0091859f  'pause1'
  0x0099F35C  at 009185f4  'pause1'
direct callees (4):
  0x0076D200  CSWCCreature::SetAppearance
  0x007ED830  CSWCObject::GetAnimationBase
  0x00414840  CSWGuiControl image setter thiscall(CResRef* name, int force) RET 8; current resref at ctl+0x44; work only when name differs (0x710A70) or force
  0x009196EE  __security_check_cookie
struct offsets touched (top 10):
  +0x68 [dec 104]  x14  via EAX,ECX,EDX
  +0x1CEC [dec 7404]  x4  via EAX,ECX,EDX
  +0x8 [dec 8]  x3  via EAX
  +0x18 [dec 24]  x3  via ECX,EDX
  +0x224 [dec 548]  x2  via EDX
  +0x1CC4 [dec 7364]  x2  via EAX
  +0x1CD0 [dec 7376]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via EAX
  +0x48 [dec 72]  x1  via ECX
  +0x80 [dec 128]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00918420 FUN_00918420 size=774 ret=0 callers=4 callees=4

void __fastcall FUN_00918420(int param_1)

{
  int *piVar1;
  undefined4 uVar2;
  int iVar3;
  undefined4 *puVar4;
  undefined4 *puVar5;
  float10 fVar6;
  undefined1 local_5c [16];
  int local_4c;
  int *local_48;
  undefined4 local_44[6 = byte +0x18];
  undefined2 local_2c;
  uint local_8;
  
  local_8 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  if ((uint)*(ushort *)(*(int *)(*(int *)(param_1 + 0x68) + 0x310) + 0x124) !=
      *(uint *)(*(int *)(param_1 + 0x1cc4) + *(int *)(param_1 + 0x1cec) * 4)) {
    (**(code **)(**(int **)(param_1 + 0x68) + 0xf4))
              (*(undefined2 *)(*(int *)(param_1 + 0x1cc4) + *(int *)(param_1 + 0x1cec) * 4));
    puVar4 = *(undefined4 **)(*(int *)(param_1 + 0x68) + 0x224);
    puVar5 = local_44;
    for (iVar3 = 0xf; iVar3 != 0; iVar3 = iVar3 + -1) {
      *puVar5 = *puVar4;
      puVar4 = puVar4 + 0x1 [dec 1] {undefined4*: byte +0x4};
      puVar5 = puVar5 + 0x1 [dec 1] {undefined4*: byte +0x4};
    }
    local_2c = *(undefined2 *)(*(int *)(param_1 + 0x1cd0) + *(int *)(param_1 + 0x1cec) * 4);
    FUN_0076d200(local_44,2,1,1);
    *(undefined2 *)(*(int *)(*(int *)(param_1 + 0x68) + 0x224) + 0x18) =
         *(undefined2 *)(*(int *)(param_1 + 0x1cd0) + *(int *)(param_1 + 0x1cec) * 4);
    if (DAT_009f42b4 != 0) {
      piVar1 = (int *)FUN_007ed830();
      local_48 = (int *)(**(code **)(*piVar1 + 0x8 [dec 8] {int*: byte +0x20}))(0xff);
      if (local_48 != (int *)0x0) {
        (**(code **)(*local_48 + 0x18))("pause1",0x3f800000,2,0);
      }
      piVar1 = (int *)FUN_007ed830();
      local_48 = (int *)(**(code **)(*piVar1 + 0x8 [dec 8] {int*: byte +0x20}))(0xfe);
      if (local_48 != (int *)0x0) {
        (**(code **)(*local_48 + 0x18))("pause1",0x3f800000,2,0);
      }
    }
  }
  uVar2 = (**(code **)(**(int **)(param_1 + 0x68) + 0x148))(local_5c,0);
  FUN_00414840(uVar2,0);
  if (DAT_009f42b4 != 0) {
    (**(code **)(**(int **)(param_1 + 0x68) + 0x94))(*(undefined4 *)(param_1 + 0xb24));
    local_4c = (**(code **)(**(int **)(param_1 + 0xb28) + 0x80))(0x3ea);
    if (local_4c != 0) {
      *(undefined4 *)(local_4c + 0x48) = 0;
      fVar6 = (float10)(**(code **)(**(int **)(param_1 + 0x68) + 0xd8))();
      *(undefined4 *)(local_4c + 0x3c) = 0;
      *(float *)(local_4c + 0x40) = (float)fVar6;
      *(undefined4 *)(local_4c + 0x44) = 0;
    }
  }
  __security_check_cookie [GHIDRA-AUTO-LABEL: unreliable](local_8 ^ (uint)&stack0xfffffffc);
  return;
}
```

### decoy: 0x00918030 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00918030: 16-byte aligned: YES
  00918030  55             PUSH EBP
  00918031  8B EC          MOV EBP,ESP
  00918033  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00918030
string literals (7):
  0x0099CF54  at 00918085  'PORTRAITS'
  0x009A9B3C  at 0091817b  'ForPC'
  0x009A9B00  at 00918209  'Appearance_S'
  0x009A9AEC  at 0091823f  'AppearanceNumber'
  0x009A9ADC  at 00918272  'Appearance_L'
  0x009A9AEC  at 009182a5  'AppearanceNumber'
  0x00994678  at 009182df  'BaseResRef'
direct callees (14):
  0x00919723  operator new
  0x00710840  (unnamed)
  0x0071AD40  CRes2DA helper ctor (name, flag)
  0x0071C3D0  C2DA::LoadText
  0x00733540  CExoString::Constructor
  0x007107F0  (unnamed)
  0x006169E0  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x00844E40  CSWSCreatureStats::GetClassByPosition
  0x0071B1D0  (unnamed)
  0x0083EA60  CExoArrayList<uint32>::Add
  0x009196EE  __security_check_cookie
struct offsets touched (top 6):
  +0x68 [dec 104]  x2  via ECX,EDX
  +0x310 [dec 784]  x2  via EAX,EDX
  +0x24 [dec 36]  x1  via EDX
  +0x124 [dec 292]  x1  via EDX
  +0x1CC8 [dec 7368]  x1  via EDX
  +0x1CEC [dec 7404]  x1  via ECX
```

### Static facts (script output, report-only)
- size 774 B, RET 0
- callers 4: 0x00916ef0, 0x00917d60, 0x00917e20, 0x00917e80
- callees: 0x0076d200 (CALL), 0x007ed830 (CALL), 0x00414840 CSWGuiBorderParams::SetFillImage (CALL), 0x009196ee __security_check_cookie (CALL)
- indirect: 0x009184b9 slot 0xf4, 0x00918586 slot 0x8, 0x009185af slot 0x18, 0x009185db slot 0x8, 0x00918604 slot 0x18, ...
- strings: 0x0099f35c "pause1"
- twins: none

## Card B17-6-02

GOG function: CTlkTable::ClearCustomTokens  gog 0x00628ac0  callconv=(blank)  param_size_bytes=0  gog_size=76  gog_retn=0
Candidate Steam address: 0x0071eb80 (size 76, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.67) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callers: UnloadModule 0x5315e0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0071e910:STR0/CALLEE0/SIG0.417/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00628850 CTlkTable::SetCustomToken  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00628b10 CTlkTable::ParseStr -> 0x0071ebd0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00627790 CTlkTableCustomToken::`vector -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x007b4960 CServerExoAppInternal::UnloadModule -> Steam 0x005315e0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x48*,0x4c*} | Steam {0x48*,0x4c*}
imm sets (* = in both, 2 common): GOG {0x3*,0x10*} | Steam {0x3*,0x10*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0071eb80: 16-byte aligned: YES
  0071eb80  55             PUSH EBP
  0071eb81  8B EC          MOV EBP,ESP
  0071eb83  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0071eb80
string literals (0):
direct callees (1):
  0x0071D830  (unnamed)
struct offsets touched (top 2):
  +0x48 [dec 72]  x2  via EAX
  +0x4C [dec 76]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0071eb80 FUN_0071eb80 size=76 ret=0 callers=1 callees=1

void __fastcall FUN_0071eb80(int param_1)

{
  if (*(int *)(param_1 + 0x48) != 0) {
    FUN_0071d830(3);
  }
  *(undefined4 *)(param_1 + 0x48) = 0;
  *(undefined4 *)(param_1 + 0x4c) = 0;
  return;
}
```

### decoy: 0x0071e910 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0071e910: 16-byte aligned: YES
  0071e910  55             PUSH EBP
  0071e911  8B EC          MOV EBP,ESP
  0071e913  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0071e910
string literals (0):
direct callees (4):
  0x007337C0  CExoString::operator=
  0x00919723  operator new
  0x0091D67E  (unnamed)
  0x0071D830  (unnamed)
struct offsets touched (top 2):
  +0x48 [dec 72]  x8  via EAX,ECX,EDX
  +0x4C [dec 76]  x5  via EAX,ECX,EDX
```

### Static facts (script output, report-only)
- size 76 B, RET 0
- callers 1: 0x005315e0 CServerExoAppInternal::UnloadModule
- callees: 0x0071d830 (CALL)
- twins: none

## Card B17-6-03

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

## Card B17-6-04

GOG function: CTlkTable::ParseStr  gog 0x00628b10  callconv=__thiscall  param_size_bytes=4  gog_size=2040  gog_retn=4
Candidate Steam address: 0x0071ebd0 (size 2040, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.85) | G8 mac_rotation: pass
- anchors: A-STR 2 | A-CALLEE 14 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,CALLEE,SIG  (content level C2)
- evidence summary: STR 'CUSTOM'@0099c4fc; '<UNRECOGNIZED TOKEN>'@0099c504; CALLEE 14 (callees: Constructor_4 0x733680; callers: CreateUsableItemEntry 0x77bcf0, GetGUIString 0x79e210, ? 0x7cbf60); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0071eb80:STR0/CALLEE0/SIG0.168/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00628ac0 CTlkTable::ClearCustomTokens -> 0x0071eb80  gate_class=REVIEW
- GOG-order neighbour after:  0x00629310 CTlkTable::GetLanguageVersion  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (13), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  0x00605790 CExoString::Constructor_4 -> Steam 0x00733680  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x006059e0 CExoString::operator=_2 -> Steam ?  in candidate callees: n/a
  0x00605f00 CExoString::operator+ -> Steam 0x00733df0  in candidate callees: yes
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  ... 5 more GOG callees
GOG callers mapped through trusted rows (13 of 47): the card's A-CALLEE credit may come from these, not from callees
  0x00427c70 CSWCCreature::CreateUsableItemEntry -> Steam 0x0077bcf0  in candidate callers: yes
  0x0044a810 CClientExoAppInternal::GetGUIString -> Steam 0x0079e210  in candidate callers: yes
  0x004dc660 CGuiInGame::HandleDialogEntry -> Steam 0x007cbf60  in candidate callers: yes
  0x004dccc0 CGuiInGame::HandleDialogReplies -> Steam 0x007cc5c0  in candidate callers: yes
GOG strings (2): 'CUSTOM'; '<UNRECOGNIZED TOKEN>'
disp sets (* = in both, 7 common): GOG {0x1*,0x4*,0x40*,0x44*,0x48*,0x4c*,0x50*} | Steam {0x1*,0x4*,0x40*,0x44*,0x48*,0x4c*,0x50*}
imm sets (* = in both, 19 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0x24*,0x30*,0x39*,...(+5)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0x24*,0x30*,0x39*,...(+5)}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0071ebd0: 16-byte aligned: YES
  0071ebd0  55             PUSH EBP
  0071ebd1  8B EC          MOV EBP,ESP
  0071ebd3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0071ebd0
string literals (2):
  0x0099C504  at 0071edcf  '<UNRECOGNIZED TOKEN>'
  0x0099C4FC  at 0071ee61  'CUSTOM'
direct callees (13):
  0x00733540  CExoString::Constructor
  0x00734010  CExoString::CStr
  0x0071D930  (unnamed)
  0x00733680  CExoString::Constructor_4
  0x00733DF0  CExoString::operator+
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x007338D0  CExoString::operator=_2
  0x0091AC7C  (unnamed)
  0x00722E50  (unnamed)
  0x00919F10  _strlen
  0x0091CF23  (unnamed)
  0x00733570  CExoString::CExoString_char
struct offsets touched (top 6):
  +0x50 [dec 80]  x6  via EAX,ECX
  +0x40 [dec 64]  x5  via EAX,ECX,EDX
  +0x48 [dec 72]  x3  via EAX,ECX,EDX
  +0x1 [dec 1]  x2  via ECX
  +0x44 [dec 68]  x1  via EDX
  +0x4C [dec 76]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0071ebd0 FUN_0071ebd0 size=2040 ret=4 callers=47 callees=13

/* WARNING: Removing unreachable block (ram,0x0071ee0e) */
/* WARNING: Removing unreachable block (ram,0x0071ee27) */
/* WARNING: Removing unreachable block (ram,0x0071ec5f) */
/* WARNING: Removing unreachable block (ram,0x0071ec78) */
/* WARNING: Removing unreachable block (ram,0x0071f2a7) */
/* WARNING: Removing unreachable block (ram,0x0071f2c0) */

void __fastcall FUN_0071ebd0(undefined4 *param_1)

{
  int *piVar1;
  uint uVar2;
  undefined4 uVar3;
  int iVar4;
  size_t _MaxCount [GHIDRA-AUTO-LABEL: unreliable];
  char *_Str1 [GHIDRA-AUTO-LABEL: unreliable];
  char *_Str2 [GHIDRA-AUTO-LABEL: unreliable];
  size_t local_ac;
  undefined1 local_a4 [16];
  undefined1 local_94 [16];
  undefined1 local_84 [16];
  undefined1 local_74 [8];
  undefined1 local_6c [16];
  undefined1 local_5c [16];
  int local_4c;
  int local_48;
  undefined1 local_44 [8];
  undefined1 local_3c [8];
  uint local_34;
  int local_30;
  int local_2c;
  int local_28;
  uint local_24;
  int local_20;
  uint local_1c;
  uint local_18;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009602b4;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  FUN_00733540();
  local_8 = CONCAT31(local_8._1_3_,1);
  local_4c = 0;
  param_1[0x14 = byte +0x50] = 0;
  local_2c = FUN_00734010();
  local_20 = 0;
  do {
    if (*(char *)(local_2c + local_20) == '\0') {
      if (0 < local_4c) {
        if ((local_4c < local_20) && (param_1[0x14 = byte +0x50] == 0)) {
          uVar3 = FUN_00733570(local_2c + local_4c);
          local_8._0_1_ = 0xb;
          uVar3 = FUN_00733df0(local_a4,uVar3);
          local_8._0_1_ = 0xc;
          FUN_007337c0(uVar3);
          local_8._0_1_ = 0xb;
          FUN_00733780();
          local_8 = CONCAT31(local_8._1_3_,1);
          FUN_00733780();
        }
        FUN_007337c0(local_44);
      }
      local_8 = local_8 & 0xffffff00;
      FUN_00733780();
      local_8 = 0xffffffff;
      FUN_00733780();
      ExceptionList = local_10;
      return;
    }
    if (*(char *)(local_2c + local_20) == '<') {
      if (*(char *)(local_2c + local_20 + 1) == '<') {
        local_20 = local_20 + 1;
/* ... 145 more lines truncated by --max-lines */
```

### decoy: 0x0071eb80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0071eb80: 16-byte aligned: YES
  0071eb80  55             PUSH EBP
  0071eb81  8B EC          MOV EBP,ESP
  0071eb83  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0071eb80
string literals (0):
direct callees (1):
  0x0071D830  (unnamed)
struct offsets touched (top 2):
  +0x48 [dec 72]  x2  via EAX
  +0x4C [dec 76]  x1  via ECX
```

### Static facts (script output, report-only)
- size 2040 B, RET 4
- callers 47: 0x0071d970, 0x007358d0, 0x00742750, 0x00756b70, 0x00758e30, ...
- callees: 0x00733540 CExoString::DefaultConstructor (CALL), 0x00734010 (CALL), 0x0071d930 (CALL), 0x00733680 CExoString::CStrLenConstructor (CALL), 0x00733df0 CExoString::operator+ (CALL), 0x007337c0 CExoString::operator= (CALL), ...
- indirect: 0x0071f0be slot 0x0
- strings: 0x0099c4fc "CUSTOM", 0x0099c504 "<UNRECOGNIZED TOKEN>"
- twins: none

## Card B17-6-05

GOG function: CSWGuiScene::Draw  gog 0x009168b0  callconv=(blank)  param_size_bytes=4  gog_size=120  gog_retn=4
Candidate Steam address: 0x004185d0 (size 36, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWGuiControl::SetActive | G7 mac: n/a | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.12 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00418600:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00916870 CSWGuiScene::GetModel  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00916930 CSWGuiNavigable::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x008cfd70 Global::AurGUISetupViewport -> Steam ?  in candidate callees: n/a
  0x008d02d0 Global::AurGUICloseViewport -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x8,0xc,0x10,0x14,0x18,0x1c} | Steam {0x48}
imm sets (* = in both, 1 common): GOG {0x4*,0x1c,0x28} | Steam {0x4*,0xfffffffe}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x004185d0: 16-byte aligned: YES
  004185d0  55             PUSH EBP
  004185d1  8B EC          MOV EBP,ESP
  004185d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004185d0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x48 [dec 72]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 004185d0 FUN_004185d0 size=36 ret=4 callers=0 callees=0

void __thiscall FUN_004185d0(int param_1,uint param_2)

{
  *(uint *)(param_1 + 0x48) = *(uint *)(param_1 + 0x48) & 0xfffffffe | param_2 & 1;
  return;
}
```

### decoy: 0x00418600 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00418600: 16-byte aligned: YES
  00418600  55             PUSH EBP
  00418601  8B EC          MOV EBP,ESP
  00418603  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00418600
string literals (0):
direct callees (2):
  0x00418640  CSWGuiControl::Destructor
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 36 B, RET 4
- twins: none

## Card B17-6-06

GOG function: CSWPlayerControlCamRelative::GetPlayerWalking  gog 0x004f0520  callconv=__thiscall  param_size_bytes=0  gog_size=17  gog_retn=0
Candidate Steam address: 0x00726fa0 (size 105, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00726f80:STR0/CALLEE17/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004f0500 CSWPlayerControlCamRelative::SetPlayerWalking  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004f0540 CSWPlayerControlCamRelative::GetCurSpeed  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (18 of 20): the card's A-CALLEE credit may come from these, not from callees
  0x0040c8f0 CClientExoApp::GetModule -> Steam 0x0073f430  in candidate callers: no
  0x0042dd90 CClientExoAppInternal::GetModuleCamera -> Steam 0x00781810  in candidate callers: no
  0x0042e160 CClientExoAppInternal::MainLoop -> Steam 0x00781be0  in candidate callers: no
  0x00430340 CClientExoAppInternal::UpdateRoom -> Steam 0x00783dc0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x18} | Steam {0x1c}
imm sets (* = in both, 0 common): GOG {} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00726fa0: 16-byte aligned: YES
  00726fa0  55             PUSH EBP
  00726fa1  8B EC          MOV EBP,ESP
  00726fa3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00726fa0
string literals (0):
direct callees (2):
  0x00726ED0  (unnamed)
  0x007273C0  (unnamed)
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00726fa0 FUN_00726fa0 size=105 ret=0 callers=1 callees=2

undefined4 * __fastcall FUN_00726fa0(undefined4 *param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00960d68;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00726ed0(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  *param_1 = CExoResFile::vftable;
  param_1[7 = byte +0x1c] = 0;
  FUN_007273c0();
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x00726f80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00726f80: 16-byte aligned: YES
  00726f80  55             PUSH EBP
  00726f81  8B EC          MOV EBP,ESP
  00726f83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00726f80
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x18 [dec 24]  x1  via EAX
```

### Static facts (script output, report-only)
- size 105 B, RET 0
- callers 1: 0x00724f00
- callees: 0x00726ed0 (CALL), 0x007273c0 (CALL)
- twins: none

## Card B17-6-07

GOG function: C2DA::SkipNewLines  gog 0x00622a20  callconv=(blank)  param_size_bytes=8  gog_size=273  gog_retn=8
Candidate Steam address: 0x0071cf80 (size 273, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x0071c3d0:STR0/CALLEE0/SIG0.091/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00621e70 C2DA::Load2DArray  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00622b40 C2DA::Unload2DArray  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 6 common): GOG {0x8*,0x9*,0xa*,0xd*,0x10*,0x20*} | Steam {0x8*,0x9*,0xa*,0xd*,0x10*,0x20*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0071cf80: 16-byte aligned: YES
  0071cf80  55             PUSH EBP
  0071cf81  8B EC          MOV EBP,ESP
  0071cf83  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0071cf80
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0071cf80 FUN_0071cf80 size=273 ret=8 callers=1 callees=0

bool FUN_0071cf80(int *param_1,uint *param_2)

{
  uint local_10;
  uint local_c;
  char *local_8;
  
  local_c = 0;
  local_10 = 0;
  local_8 = (char *)*param_1;
  do {
    if ((*param_2 <= local_c) ||
       ((((*local_8 != '\n' && (*local_8 != '\r')) && (*local_8 != ' ')) && (*local_8 != '\t'))))
    goto LAB_0071d068;
    local_c = local_10;
    for (; (local_c < *param_2 && ((*local_8 == '\n' || (*local_8 == '\r')))); local_8 = local_8 + 1
        ) {
      local_c = local_c + 1;
    }
    local_10 = local_c;
    for (; (local_10 < *param_2 && ((*local_8 == ' ' || (*local_8 == '\t')))); local_8 = local_8 + 1
        ) {
      local_10 = local_10 + 1;
    }
  } while (local_10 != *param_2);
  local_c = local_10;
LAB_0071d068:
  *param_1 = *param_1 + local_c;
  *param_2 = *param_2 - local_c;
  return local_c != 0;
}
```

### decoy: 0x0071c3d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0071c3d0: 16-byte aligned: YES
  0071c3d0  55             PUSH EBP
  0071c3d1  8B EC          MOV EBP,ESP
  0071c3d3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0071c3d0
string literals (2):
  0x0099C47C  at 0071c4cd  'DEFAULT:'
  0x0099423C  at 0071c52f  'DEFAULT'
direct callees (17):
  0x0071D0A0  C2DA::Free
  0x007111E0  CRes::Demand
  0x006F3D90  (unnamed)
  0x00703BD0  (unnamed)
  0x0071CF80  (unnamed)
  0x0071C130  C2DA::GetNextLineLength
  0x0071C180  C2DA::NextToken
  0x00946489  (unnamed)
  0x0091D780  (unnamed)
  0x007338D0  CExoString::operator=_2
  0x00919F10  _strlen
  0x007112B0  CRes::Release
  0x00919723  operator new
  0x0091D67E  (unnamed)
  0x00946714  (unnamed)
  0x007337C0  CExoString::operator=
  0x009196EE  __security_check_cookie
struct offsets touched (top 10):
  +0x24 [dec 36]  x11  via EAX,ECX,EDX
  +0x28 [dec 40]  x11  via EAX,ECX,EDX
  +0x2C [dec 44]  x8  via EAX,ECX,EDX
  +0x8 [dec 8]  x7  via EAX,ECX,EDX
  +0x38 [dec 56]  x7  via EAX,ECX,EDX
  +0x30 [dec 48]  x4  via EAX,ECX,EDX
  +0x34 [dec 52]  x3  via EAX,ECX
  +0x3C [dec 60]  x2  via EAX
  +0x4C [dec 76]  x2  via EAX,EDX
  +0x50 [dec 80]  x2  via EAX,EDX
```

### Static facts (script output, report-only)
- size 273 B, RET 8
- callers 1: 0x0071c3d0
- twins: none

## Card B17-6-08

GOG function: CExoRawInputInternal::Constructor  gog 0x00617350  callconv=(blank)  param_size_bytes=8  gog_size=1847  gog_retn=8
Candidate Steam address: 0x00730750 (size 1847, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00730e90:STR0/CALLEE0/SIG0.155/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00617310 CExoDeviceBuffer::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00617a90 CExoRawInputInternal::CheckControllerType  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x00608350 CExoTimers::GetHighResolutionTimer -> Steam ?  in candidate callees: n/a
  0x00617a90 CExoRawInputInternal::CheckControllerType -> Steam ?  in candidate callees: n/a
  0x00619e00 CExoRawInputInternal::ShutDownDirectInput -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
  0x00924260 Global::memset -> Steam ?  in candidate callees: n/a
  0x00924e50 unnamed -> Steam ?  in candidate callees: n/a
  0x00943360 DINPUT8_DLL::DirectInput8Create -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 19 common): GOG {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x1c*,0x1d*,0x20*,0x24*,0x28*,0x2c*,0x30*,0x34*,...(+5)} | Steam {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x1c*,0x1d*,0x20*,0x24*,0x28*,0x2c*,0x30*,0x34*,...(+5)}
imm sets (* = in both, 17 common): GOG {0x4*,0x6*,0x8*,0xa*,0xc*,0x10*,0x14*,0x18*,0x20*,0x24*,0x74*,0x100*,0x110*,0x134*,...(+3)} | Steam {0x4*,0x6*,0x8*,0xa*,0xc*,0x10*,0x14*,0x18*,0x20*,0x24*,0x74*,0x100*,0x110*,0x134*,...(+3)}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00730750: 16-byte aligned: YES
  00730750  55             PUSH EBP
  00730751  8B EC          MOV EBP,ESP
  00730753  83 EC 74       SUB ESP,0x74
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00730750
string literals (0):
direct callees (7):
  0x0093B3B0  (unnamed)
  0x00733200  (unnamed)
  0x00919723  operator new
  0x00730E90  (unnamed)
  0x0091C630  (unnamed)
  0x00736220  CExoTimers::GetHighResolutionTimer
  0x0091D220  (unnamed)
struct offsets touched (top 10):
  +0x2C [dec 44]  x28  via EAX,ECX,EDX
  +0x18 [dec 24]  x10  via EAX,ECX,EDX
  +0x34 [dec 52]  x10  via EAX,EDX
  +0x30 [dec 48]  x8  via EAX,ECX,EDX
  +0x20 [dec 32]  x7  via EAX,ECX,EDX
  +0x24 [dec 36]  x7  via EAX,ECX,EDX
  +0x14 [dec 20]  x6  via ECX
  +0x4 [dec 4]  x3  via EAX
  +0x8 [dec 8]  x3  via ECX
  +0x10 [dec 16]  x3  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00730750 FUN_00730750 size=1847 ret=8 callers=1 callees=7

undefined4 * __thiscall FUN_00730750(undefined4 *param_1,undefined4 param_2,undefined4 param_3)

{
  undefined4 uVar1;
  undefined8 uVar2;
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  int local_3c;
  int local_38;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  int local_c;
  int local_8;
  
  *(undefined1 *)(param_1 + 0x7 [dec 7] {undefined4*: byte +0x1c}) = 0;
  *(undefined1 *)((int)param_1 + 0x1d) = 0;
  *param_1 = 0;
  param_1[8 = byte +0x20] = 0;
  param_1[9 = byte +0x24] = 0;
  param_1[10 = byte +0x28] = 0;
  param_1[1 = byte +0x4] = 0;
  param_1[2 = byte +0x8] = 0;
  param_1[3 = byte +0xc] = 1;
  param_1[4 = byte +0x10] = 0;
  param_1[5 = byte +0x14] = 0;
  local_c = DirectInput8Create(param_2,0x800,&DAT_0099cac8,param_1 + 0x8 [dec 8] {undefined4*: byte +0x20},0);
  if ((local_c == 0) ||
     (local_c = DirectInput8Create(param_2,0x300,&DAT_0099cac8,param_1 + 0x8 [dec 8] {undefined4*: byte +0x20},0), local_c == 0)) {
    local_c = (**(code **)(*(int *)param_1[8 = byte +0x20] + 0xc))(param_1[8 = byte +0x20],&DAT_0099cb88,param_1 + 0x9 [dec 9] {undefined4*: byte +0x24},0);
    if (local_c == 0) {
      local_c = (**(code **)(*(int *)param_1[9 = byte +0x24] + 0x2c))(param_1[9 = byte +0x24],&DAT_009b932c);
      if (local_c == 0) {
        local_c = (**(code **)(*(int *)param_1[9 = byte +0x24] + 0x34))(param_1[9 = byte +0x24],param_3,6);
        if (local_c == 0) {
          local_20 = 0x14;
          local_1c = 0x10;
          local_18 = 0;
          local_14 = 0;
          local_10 = 0x100;
          local_c = (**(code **)(*(int *)param_1[9 = byte +0x24] + 0x18))(param_1[9 = byte +0x24],1,&local_20);
          if (local_c == 0) {
            param_1[6 = byte +0x18] = 0;
            (**(code **)(*(int *)param_1[8 = byte +0x20] + 0x10))(param_1[8 = byte +0x20],4,FUN_007306a0,param_1,1);
            param_1[0xb = byte +0x2c] = 0;
            param_1[0xc = byte +0x30] = 0;
            param_1[0xd = byte +0x34] = 0;
            if (param_1[6 = byte +0x18] != 0) {
              uVar1 = FUN_00919723(-(uint)((int)((ulonglong)(uint)param_1[6 = byte +0x18] * 4 >> 0x20) != 0) |
                                   (uint)((ulonglong)(uint)param_1[6 = byte +0x18] * 4));
              param_1[0xb = byte +0x2c] = uVar1;
            }
            for (local_8 = 0; local_8 < (int)param_1[6 = byte +0x18]; local_8 = local_8 + 1) {
              *(undefined4 *)(param_1[0xb = byte +0x2c] + local_8 * 4) = 0;
            }
            if (param_1[6 = byte +0x18] != 0) {
              uVar1 = FUN_00919723(-(uint)((int)((ulonglong)(uint)param_1[6 = byte +0x18] * 0x134 >> 0x20) != 0)
                                   | (uint)((ulonglong)(uint)param_1[6 = byte +0x18] * 0x134));
              param_1[0xc = byte +0x30] = uVar1;
              uVar1 = FUN_00919723(-(uint)((int)((ulonglong)(uint)param_1[6 = byte +0x18] * 0x134 >> 0x20) != 0)
                                   | (uint)((ulonglong)(uint)param_1[6 = byte +0x18] * 0x134));
              param_1[0xd = byte +0x34] = uVar1;
            }
            DAT_00a7ea6c = 0;
            (**(code **)(*(int *)param_1[8 = byte +0x20] + 0x10))(param_1[8 = byte +0x20],4,FUN_007306c0,param_1,1);
            param_1[6 = byte +0x18] = DAT_00a7ea6c;
            for (local_8 = 0; local_8 < (int)param_1[6 = byte +0x18]; local_8 = local_8 + 1) {
              if ((*(int *)(param_1[0xb = byte +0x2c] + local_8 * 4) != 0) &&
                 (local_c = (**(code **)(**(int **)(param_1[0xb = byte +0x2c] + local_8 * 4) + 0x2c))
                                      (*(undefined4 *)(param_1[0xb = byte +0x2c] + local_8 * 4),&DAT_009b9124),
                 local_c != 0)) {
/* ... 89 more lines truncated by --max-lines */
```

### decoy: 0x00730e90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00730e90: 16-byte aligned: YES
  00730e90  55             PUSH EBP
  00730e91  8B EC          MOV EBP,ESP
  00730e93  81 EC 68 02 00 00 SUB ESP,0x268
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00730e90
string literals (1):
  0x0099CB98  at 00730f82  'playstation'
direct callees (5):
  0x0091E01A  (unnamed)
  0x0091D6F0  (unnamed)
  0x0091F94D  (unnamed)
  0x0091AC7C  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 4):
  +0x1D [dec 29]  x3  via EAX,ECX
  +0x1C [dec 28]  x2  via EAX,ECX
  +0x2C [dec 44]  x2  via EAX,ECX
  +0x3C [dec 60]  x1  via EAX
```

### Static facts (script output, report-only)
- size 1847 B, RET 8
- callers 1: 0x0072a130
- callees: 0x00733200 (CALL), 0x00919723 operator new (CALL), 0x00730e90 (CALL), 0x0091c630 (CALL), 0x00736220 (CALL), 0x0091d220 (CALL)
- indirect: 0x0073083d slot 0xc, 0x00730872 slot 0x2c, 0x007308a8 slot 0x34, 0x00730901 slot 0x18, 0x00730948 slot 0x10, ...
- globals: 0x00a1b488 READ
- imports: DirectInput8Create (DINPUT8.DLL)
- twins: none

## Card B17-6-09

GOG function: CSWGuiScene::Draw  gog 0x009168b0  callconv=(blank)  param_size_bytes=4  gog_size=120  gog_retn=4
Candidate Steam address: 0x004184b0 (size 54, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=SceneGetModel | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.19 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004184f0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00916870 CSWGuiScene::GetModel  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00916930 CSWGuiNavigable::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x008cfd70 Global::AurGUISetupViewport -> Steam ?  in candidate callees: n/a
  0x008d02d0 Global::AurGUICloseViewport -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4,0x8,0xc,0x10,0x14,0x18,0x1c*} | Steam {0x1c*,0x20}
imm sets (* = in both, 1 common): GOG {0x4*,0x1c,0x28} | Steam {0x4*,0x8}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x004184b0: 16-byte aligned: YES
  004184b0  55             PUSH EBP
  004184b1  8B EC          MOV EBP,ESP
  004184b3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004184b0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x1C [dec 28]  x1  via EAX
  +0x20 [dec 32]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 004184b0 FUN_004184b0 size=54 ret=4 callers=3 callees=0

undefined4 __thiscall FUN_004184b0(int param_1,int param_2)

{
  undefined4 uVar1;
  
  if ((param_2 < 0) || (*(int *)(param_1 + 0x20) <= param_2)) {
    uVar1 = 0;
  }
  else {
    uVar1 = *(undefined4 *)(*(int *)(param_1 + 0x1c) + param_2 * 4);
  }
  return uVar1;
}
```

### decoy: 0x004184f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004184f0: 16-byte aligned: YES
  004184f0  55             PUSH EBP
  004184f1  8B EC          MOV EBP,ESP
  004184f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004184f0
string literals (0):
direct callees (2):
  0x00479460  GUI fill rect (glViewport + coloured unit quad; returns 0 if rejected)
  0x004799C0  (unnamed)
struct offsets touched (top 7):
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x14 [dec 20]  x2  via EAX,EDX
  +0x18 [dec 24]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via EDX
  +0xC [dec 12]  x1  via ECX
  +0x10 [dec 16]  x1  via EDX
  +0x1C [dec 28]  x1  via EDX
```

### Static facts (script output, report-only)
- size 54 B, RET 4
- callers 3: 0x0077ec20, 0x008cce70 CSWGuiUpgrade::Draw, 0x008cd2c0 CSWGuiUpgrade::Update3DView
- twins: none

## Card B17-6-10

GOG function: CExoKeyTable::GetTableIndex  gog 0x00630410  callconv=(blank)  param_size_bytes=6  gog_size=142  gog_retn=8
Candidate Steam address: 0x007269c0 (size 142, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.67) | G8 mac_rotation: rotation:0x00630390:1.00>0.57
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00726940:STR0/CALLEE0/SIG0.229/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00630390 CExoKeyTable::FindKey_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x006304a0 CExoKeyTable::RebuildTable  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00928a62 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xc*} | Steam {0xc*}
imm sets (* = in both, 6 common): GOG {0x4*,0x8*,0x10*,0x14*,0xff*,0xffffff*} | Steam {0x4*,0x8*,0x10*,0x14*,0xff*,0xffffff*}

### abi
```
GOG param bytes: 6; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(6/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 6 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x007269c0: 16-byte aligned: YES
  007269c0  55             PUSH EBP
  007269c1  8B EC          MOV EBP,ESP
  007269c3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007269c0
string literals (0):
direct callees (1):
  0x0091E01A  (unnamed)
struct offsets touched (top 1):
  +0xC [dec 12]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007269c0 FUN_007269c0 size=142 ret=8 callers=2 callees=1

uint __thiscall FUN_007269c0(int param_1,int param_2,ushort param_3)

{
  int iVar1;
  uint local_14;
  uint local_8;
  
  local_14 = 0;
  for (local_8 = 0; (local_8 < 0x10 && (*(char *)(param_2 + local_8) != '\0'));
      local_8 = local_8 + 1) {
    iVar1 = _tolower [GHIDRA-AUTO-LABEL: unreliable]((uint)*(byte *)(param_2 + local_8));
    local_14 = local_14 >> 8 ^
               *(uint *)(&DAT_00a7e5a0 + ((iVar1 + (uint)param_3 ^ local_14) & 0xff) * 4);
  }
  return local_14 % *(uint *)(param_1 + 0xc);
}
```

### decoy: 0x00726940 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00726940: 16-byte aligned: YES
  00726940  55             PUSH EBP
  00726941  8B EC          MOV EBP,ESP
  00726943  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00726940
string literals (0):
direct callees (1):
  0x00710B40  (unnamed)
struct offsets touched (top 2):
  +0x10 [dec 16]  x3  via EAX,EDX
  +0xC [dec 12]  x1  via EDX
```

### Static facts (script output, report-only)
- size 142 B, RET 8
- callers 2: 0x00724b50, 0x00726670
- callees: 0x0091e01a (CALL)
- twins: none

## Card B17-6-11

GOG function: CRes::Dump  gog 0x0061ab90  callconv=(blank)  param_size_bytes=4  gog_size=32  gog_retn=4
Candidate Steam address: 0x00711230 (size 32, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (callees: Dump 0x711e60; callers: DestroyTable 0x726350)
- best decoy (neighbour/other candidate) scores: 0x00711200:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061ab60 CRes::ReadRaw  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0061abb0 CRes::GetDemands  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0061b7c0 CExoResMan::Dump -> Steam 0x00711e60  in candidate callees: yes
GOG callers mapped through trusted rows (2 of 6): the card's A-CALLEE credit may come from these, not from callees
  0x0061c970 CExoResMan::ReleaseResObject -> Steam 0x00713010  in candidate callers: yes
  0x0062fda0 CExoKeyTable::DestroyTable -> Steam 0x00726350  in candidate callers: yes
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
prologue for 0x00711230: 16-byte aligned: YES
  00711230  55             PUSH EBP
  00711231  8B EC          MOV EBP,ESP
  00711233  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00711230
string literals (0):
direct callees (1):
  0x00711E60  CExoResMan::Dump
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00711230 FUN_00711230 size=32 ret=4 callers=6 callees=1
THUNK -> 0x00711E60 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __thiscall FUN_00711230(undefined4 param_1,undefined4 param_2)

{
  FUN_00711e60(param_1,param_2);
  return;
}
```

### decoy: 0x00711200 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00711200: 16-byte aligned: YES
  00711200  55             PUSH EBP
  00711201  8B EC          MOV EBP,ESP
  00711203  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00711200
string literals (0):
direct callees (1):
  0x00714580  CExoResMan::ReadRaw
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 32 B, RET 4
- callers 6: 0x005e9ee0, 0x00631260, 0x007111a0, 0x00713010 CExoResMan::ReleaseResObject, 0x00726350 CExoKeyTable::DestroyTable, ...
- callees: 0x00711e60 CExoResMan::Dump (CALL)
- globals: 0x00a1b490 READ
- twins: none

## Card B17-6-12

GOG function: CExoInputInternal::UpdateMouseDelta  gog 0x00615ec0  callconv=(blank)  param_size_bytes=0  gog_size=492  gog_retn=4
Candidate Steam address: 0x0072f110 (size 492, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0072f0f0:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00615ea0 CExoInputInternal::IsPsShowButtonsController  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x006160b0 CExoInputInternal::GetLastCapturedKeyboardKey  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00619a00 CExoRawInputInternal::GetMouseState -> Steam ?  in candidate callees: n/a
  0x00921312 Global::@__security_check_cookie@4 -> Steam ?  in candidate callees: n/a
  0x00924260 Global::memset -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x1a0*,0x408*,0x40c*} | Steam {0x1a0*,0x408*,0x40c*}
imm sets (* = in both, 6 common): GOG {0x4*,0x5*,0xc*,0x10*,0x41*,0x78*} | Steam {0x4*,0x5*,0xc*,0x10*,0x41*,0x78*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 4)
```

### prologue
```
prologue for 0x0072f110: 16-byte aligned: YES
  0072f110  55             PUSH EBP
  0072f111  8B EC          MOV EBP,ESP
  0072f113  83 EC 78       SUB ESP,0x78
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0072f110
string literals (0):
direct callees (3):
  0x0091C630  (unnamed)
  0x00732E00  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 3):
  +0x408 [dec 1032]  x2  via ECX,EDX
  +0x40C [dec 1036]  x2  via EAX,ECX
  +0x1A0 [dec 416]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0072f110 FUN_0072f110 size=492 ret=4 callers=1 callees=3

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0072f110(int param_1)

{
  int iVar1;
  double local_78;
  double local_6c;
  double local_60;
  double local_50;
  double local_44;
  double local_38;
  int local_18;
  int local_14;
  uint local_8;
  
  local_8 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  _memset [GHIDRA-AUTO-LABEL: unreliable](&local_18,0,0x10);
  iVar1 = FUN_00732e00(&local_18);
  if (iVar1 == 1) {
    if (((double)-local_18 * _DAT_00986b50) / (double)_DAT_00a1023c <= _DAT_00986b50) {
      local_38 = ((double)-local_18 * _DAT_00986b50) / (double)_DAT_00a1023c;
    }
    else {
      local_38 = 1.0;
    }
    if (_DAT_00986d98 <= local_38) {
      if (((double)-local_18 * _DAT_00986b50) / (double)_DAT_00a1023c <= _DAT_00986b50) {
        local_50 = ((double)-local_18 * _DAT_00986b50) / (double)_DAT_00a1023c;
      }
      else {
        local_50 = 1.0;
      }
      local_44 = local_50;
    }
    else {
      local_44 = _DAT_00986d98;
    }
    *(float *)(param_1 + 0x408) = (float)local_44;
    if (((double)-local_14 * _DAT_00986b50) / (double)_DAT_00a1023c <= _DAT_00986b50) {
      local_60 = ((double)-local_14 * _DAT_00986b50) / (double)_DAT_00a1023c;
    }
    else {
      local_60 = 1.0;
    }
    if (_DAT_00986d98 <= local_60) {
      if (((double)-local_14 * _DAT_00986b50) / (double)_DAT_00a1023c <= _DAT_00986b50) {
        local_78 = ((double)-local_14 * _DAT_00986b50) / (double)_DAT_00a1023c;
      }
      else {
        local_78 = 1.0;
      }
      local_6c = local_78;
    }
    else {
      local_6c = _DAT_00986d98;
    }
    *(float *)(param_1 + 0x40c) = (float)local_6c;
  }
  else {
    *(undefined4 *)(param_1 + 0x408) = 0;
    *(undefined4 *)(param_1 + 0x40c) = 0;
  }
  __security_check_cookie [GHIDRA-AUTO-LABEL: unreliable](local_8 ^ (uint)&stack0xfffffffc);
  return;
}
```

### decoy: 0x0072f0f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0072f0f0: 16-byte aligned: YES
  0072f0f0  55             PUSH EBP
  0072f0f1  8B EC          MOV EBP,ESP
  0072f0f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0072f0f0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x1D [dec 29]  x1  via ECX
  +0x1A0 [dec 416]  x1  via EAX
```

### Static facts (script output, report-only)
- size 492 B, RET 4
- callers 1: 0x0072fd90
- callees: 0x0091c630 (CALL), 0x00732e00 (CALL), 0x009196ee __security_check_cookie (CALL)
- twins: none

## Card B17-6-13

GOG function: CResRef::GetResRefStr  gog 0x0061a300  callconv=(blank)  param_size_bytes=0  gog_size=94  gog_retn=0
Candidate Steam address: 0x007109b0 (size 94, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: rotation:0x0061a280:0.96>0.72
- anchors: A-STR 0 | A-CALLEE 10 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 10 (callers: LoadModel 0x769f60, LipSync 0x77b190, LoadArea 0x7a2fb0); SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00710ca0:STR0/CALLEE4/SIG0.071/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061a2d0 CResRef::CopyToString_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0061a360 CResRef::IsValid  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00925530 Global::memcpy -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (10 of 18): the card's A-CALLEE credit may come from these, not from callees
  0x00415d80 CSWCCreature::LoadModel -> Steam 0x00769f60  in candidate callers: yes
  0x00427150 CSWCCreature::LipSync -> Steam 0x0077b190  in candidate callers: yes
  0x0044f9b0 CSWCArea::LoadArea -> Steam 0x007a2fb0  in candidate callers: yes
  0x0061d910 CExoResMan::ServiceFromResFile -> Steam 0x00713fb0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 5 common): GOG {0xc*,0x10*,0x11*,0x80000003*,0xfffffffc*} | Steam {0xc*,0x10*,0x11*,0x80000003*,0xfffffffc*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007109b0: 16-byte aligned: YES
  007109b0  55             PUSH EBP
  007109b1  8B EC          MOV EBP,ESP
  007109b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007109b0
string literals (0):
direct callees (1):
  0x0091DBA0  _memcpy
struct offsets touched (top 1):
  +0xA7E160 [dec 11002208]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007109b0 FUN_007109b0 size=94 ret=0 callers=18 callees=1

undefined * __fastcall FUN_007109b0(void *param_1)

{
  DAT_00a7e198 = DAT_00a7e198 + 1 & 0x80000003;
  if ((int)DAT_00a7e198 < 0) {
    DAT_00a7e198 = (DAT_00a7e198 - 1 | 0xfffffffc) + 1;
  }
  _memcpy [GHIDRA-AUTO-LABEL: unreliable](&DAT_00a7e150 + DAT_00a7e198 * 0x11,param_1,0x10);
  (&DAT_00a7e160)[DAT_00a7e198 * 0x11] = 0;
  return &DAT_00a7e150 + DAT_00a7e198 * 0x11;
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

### Static facts (script output, report-only)
- size 94 B, RET 0
- callers 18: 0x00413650, 0x00413a80, 0x005236b0 CSWSArea::LoadArea, 0x0055bd10 CSWSModule::SaveModuleIFOStart, 0x00572a70 CSWSCreature::LoadFromTemplate, ...
- callees: 0x0091dba0 _memcpy (CALL)
- twins: none

## Card B17-6-14

GOG function: MdlNodeTriMesh::ProcessVerts  gog 0x0083e200  callconv=(blank)  param_size_bytes=0  gog_size=2857  gog_retn=0
Candidate Steam address: 0x0044c8a0 (size 82, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Vector::operator+ | G7 mac: n/a | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.09 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0044bd70:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0083dd70 MdlNodeTriMesh::InternalGenVertices  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0083ed70 MdlNodeTriMesh::BuildVertexIndices  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x00413280 Vector::operator+ -> Steam ?  in candidate callees: n/a
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x00419020 Vector::operator* -> Steam ?  in candidate callees: n/a
  0x0080c0a0 Global::float_magnitude_Vector_const_ref -> Steam ?  in candidate callees: n/a
  0x0080c0f0 Vector::Normalize -> Steam ?  in candidate callees: n/a
  0x0081b9c0 unnamed -> Steam ?  in candidate callees: n/a
  0x0083ed30 unnamed -> Steam ?  in candidate callees: n/a
  0x00924490 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*,0xc,0x10,0x14,0x18,0x1c,0x20,0x24,0x28,0x2c,0x48,0x4c,0x58,...(+9)} | Steam {0x4*,0x8*}
imm sets (* = in both, 1 common): GOG {0x3,0x4,0x5,0x8,0xc*,0x14,0x41,0x44,0x54,0x80,0x194} | Steam {0xc*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0044c8a0: 16-byte aligned: YES
  0044c8a0  55             PUSH EBP
  0044c8a1  8B EC          MOV EBP,ESP
  0044c8a3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0044c8a0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
  +0x8 [dec 8]  x3  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0044c8a0 FUN_0044c8a0 size=82 ret=0 callers=135 callees=0

float * FUN_0044c8a0(float *param_1,float *param_2,float *param_3)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  
  fVar1 = param_3[1 = byte +0x4];
  fVar2 = param_2[1 = byte +0x4];
  fVar3 = param_3[2 = byte +0x8];
  fVar4 = param_2[2 = byte +0x8];
  *param_1 = *param_3 + *param_2;
  param_1[1 = byte +0x4] = fVar1 + fVar2;
  param_1[2 = byte +0x8] = fVar3 + fVar4;
  return param_1;
}
```

### decoy: 0x0044bd70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0044bd70: 16-byte aligned: YES
  0044bd70  55             PUSH EBP
  0044bd71  8B EC          MOV EBP,ESP
  0044bd73  81 EC 94 01 00 00 SUB ESP,0x194
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0044bd70
string literals (0):
direct callees (8):
  0x00429330  (unnamed)
  0x0091C860  _ftol2_sse
  0x0044C900  (unnamed)
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044C8A0  Vector::operator+ (Vector3 add)
  0x005142F0  (unnamed)
  0x00514340  (unnamed)
  0x00436200  (unnamed)
struct offsets touched (top 10):
  +0x134 [dec 308]  x38  via EAX,ECX,EDX
  +0x4 [dec 4]  x28  via EAX,ECX,EDX
  +0x48 [dec 72]  x25  via EAX,ECX,EDX
  +0x8 [dec 8]  x22  via EAX,ECX,EDX
  +0x4C [dec 76]  x3  via EAX,ECX
  +0x64 [dec 100]  x3  via EAX,ECX,EDX
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0x6C [dec 108]  x3  via EAX,ECX,EDX
  +0x70 [dec 112]  x3  via EAX,ECX,EDX
  +0x74 [dec 116]  x3  via EAX,ECX,EDX
```

### Static facts (script output, report-only)
- size 82 B, RET 0
- callers 135: 0x0044bd70, 0x00457510, 0x0045a510, 0x0045b1c0, 0x0045c810, ...
- twins: none

## Card B17-6-15

GOG function: CWorldTimer::GetSnapshotTime  gog 0x0080ae00  callconv=(blank)  param_size_bytes=0  gog_size=20  gog_retn=0
Candidate Steam address: 0x0051a860 (size 189, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 0.12 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051a920:STR0/CALLEE11/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0080ad40 CWorldTimer::SetSnapshotTime -> 0x0051a860  gate_class=AUTO
- GOG-order neighbour after:  0x0080ae20 CWorldTimer::GetSnapshotTimeDifference -> 0x0051a940  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (12 of 19): the card's A-CALLEE credit may come from these, not from callees
  0x00420a90 CSWCCreature::EvaluateNewWayPoints -> Steam 0x00774bc0  in candidate callers: no
  0x0042ddc0 CClientExoAppInternal::HandleMessage -> Steam 0x00781840  in candidate callers: no
  0x0042e160 CClientExoAppInternal::MainLoop -> Steam 0x00781be0  in candidate callers: yes
  0x00430d60 CClientExoAppInternal::SetPauseState -> Steam 0x00784740  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0xc*,0x10*} | Steam {0x4,0x8,0xc*,0x10*,0x14,0x18,0x1c,0x20}
imm sets (* = in both, 0 common): GOG {} | Steam {0xc,0x64,0xf4240}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051a860: 16-byte aligned: YES
  0051a860  55             PUSH EBP
  0051a861  8B EC          MOV EBP,ESP
  0051a863  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051a860
string literals (0):
direct callees (3):
  0x00736220  CExoTimers::GetHighResolutionTimer
  0x0091FC30  (unnamed)
  0x0091D220  (unnamed)
struct offsets touched (top 8):
  +0xC [dec 12]  x5  via ECX,EDX
  +0x10 [dec 16]  x5  via ECX,EDX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EDX
  +0x1C [dec 28]  x2  via EAX,ECX
  +0x20 [dec 32]  x2  via EAX,ECX
  +0x14 [dec 20]  x1  via EAX
  +0x18 [dec 24]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0051a860 FUN_0051a860 size=189 ret=0 callers=7 callees=3

void __fastcall FUN_0051a860(int *param_1)

{
  uint uVar1;
  uint uVar2;
  undefined8 uVar3;
  undefined8 uVar4;
  
  param_1[5 = byte +0x14] = param_1[3 = byte +0xc];
  param_1[6 = byte +0x18] = param_1[4 = byte +0x10];
  if (*param_1 == 0) {
    uVar3 = FUN_00736220();
    uVar4 = __allmul [GHIDRA-AUTO-LABEL: unreliable]((uint)uVar3 - param_1[7 = byte +0x1c],
                     ((int)((ulonglong)uVar3 >> 0x20) - param_1[8 = byte +0x20]) -
                     (uint)((uint)uVar3 < (uint)param_1[7 = byte +0x1c]),param_1[2 = byte +0x8],param_1[2 = byte +0x8] >> 0x1f);
    uVar4 = __aulldiv [GHIDRA-AUTO-LABEL: unreliable](uVar4,100,0);
    uVar1 = param_1[3 = byte +0xc];
    param_1[3 = byte +0xc] = (uint)uVar4 + param_1[3 = byte +0xc];
    param_1[4 = byte +0x10] = param_1[4 = byte +0x10] + (int)((ulonglong)uVar4 >> 0x20) + (uint)CARRY4((uint)uVar4,uVar1);
    *(undefined8 *)(param_1 + 0x7 [dec 7] {int*: byte +0x1c}) = uVar3;
  }
  else {
    uVar2 = ((int)(1000000 / (longlong)param_1[1 = byte +0x4]) * param_1[2 = byte +0x8]) / 100;
    uVar1 = param_1[3 = byte +0xc];
    param_1[3 = byte +0xc] = uVar2 + param_1[3 = byte +0xc];
    param_1[4 = byte +0x10] = param_1[4 = byte +0x10] + ((int)uVar2 >> 0x1f) + (uint)CARRY4(uVar2,uVar1);
  }
  return;
}
```

### decoy: 0x0051a920 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051a920: 16-byte aligned: YES
  0051a920  55             PUSH EBP
  0051a921  8B EC          MOV EBP,ESP
  0051a923  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051a920
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0xC [dec 12]  x1  via ECX
  +0x10 [dec 16]  x1  via ECX
```

### Static facts (script output, report-only)
- size 189 B, RET 0
- callers 7: 0x0051ac30, 0x00530170 CServerExoAppInternal::ExecuteLoadModule, 0x00533830 CServerExoAppInternal::MainLoop, 0x00538ed0 CServerExoAppInternal::SetPauseState, 0x00781be0 CClientExoAppInternal::MainLoop, ...
- callees: 0x00736220 (CALL), 0x0091fc30 (CALL), 0x0091d220 (CALL)
- globals: 0x00a1b488 READ
- twins: none

## Card B17-6-16

GOG function: CWorldTimer::GetSnapshotTime  gog 0x0080ae00  callconv=(blank)  param_size_bytes=0  gog_size=20  gog_retn=0
Candidate Steam address: 0x0051a940 (size 46, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 0.20 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051a920:STR0/CALLEE11/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0080ad40 CWorldTimer::SetSnapshotTime -> 0x0051a860  gate_class=AUTO
- GOG-order neighbour after:  0x0080ae20 CWorldTimer::GetSnapshotTimeDifference -> 0x0051a940  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (12 of 19): the card's A-CALLEE credit may come from these, not from callees
  0x00420a90 CSWCCreature::EvaluateNewWayPoints -> Steam 0x00774bc0  in candidate callers: no
  0x0042ddc0 CClientExoAppInternal::HandleMessage -> Steam 0x00781840  in candidate callers: no
  0x0042e160 CClientExoAppInternal::MainLoop -> Steam 0x00781be0  in candidate callers: yes
  0x00430d60 CClientExoAppInternal::SetPauseState -> Steam 0x00784740  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0xc*,0x10*} | Steam {0xc*,0x10*,0x14,0x18,0x24}
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
prologue for 0x0051a940: 16-byte aligned: YES
  0051a940  55             PUSH EBP
  0051a941  8B EC          MOV EBP,ESP
  0051a943  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051a940
string literals (0):
direct callees (0):
struct offsets touched (top 5):
  +0xC [dec 12]  x1  via ECX
  +0x10 [dec 16]  x1  via ECX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via EDX
  +0x24 [dec 36]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0051a940 FUN_0051a940 size=46 ret=0 callers=7 callees=0

undefined8 __fastcall FUN_0051a940(int param_1)

{
  int iVar1;
  int iVar2;
  
  if (*(int *)(param_1 + 0x24) == 0) {
    iVar1 = *(uint *)(param_1 + 0xc) - *(uint *)(param_1 + 0x14);
    iVar2 = (*(int *)(param_1 + 0x10) - *(int *)(param_1 + 0x18)) -
            (uint)(*(uint *)(param_1 + 0xc) < *(uint *)(param_1 + 0x14));
  }
  else {
    iVar1 = 0;
    iVar2 = 0;
  }
  return CONCAT44(iVar2,iVar1);
}
```

### decoy: 0x0051a920 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051a920: 16-byte aligned: YES
  0051a920  55             PUSH EBP
  0051a921  8B EC          MOV EBP,ESP
  0051a923  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051a920
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0xC [dec 12]  x1  via ECX
  +0x10 [dec 16]  x1  via ECX
```

### Static facts (script output, report-only)
- size 46 B, RET 0
- callers 7: 0x00781be0 CClientExoAppInternal::MainLoop, 0x0078c350 CClientExoAppInternal::UpdateCamera, 0x007a23a0, 0x007a6760 CSWCArea::Update, 0x007ac470 CClientExoAppInternal::ProcessInput, ...
- twins: none

## Card B17-6-17

GOG function: CExoInputInternal::BufferEvent  gog 0x00614de0  callconv=(blank)  param_size_bytes=4  gog_size=1743  gog_retn=4
Candidate Steam address: 0x0072e030 (size 1743, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.29) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0072e730:STR0/CALLEE0/SIG0.338/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00614db0 CExoInputInternal::UnpauseRumble  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x006154e0 CExoInputInternal::BufferEvent_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x006154e0 CExoInputInternal::BufferEvent_2 -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x4*,0x8*,0xc*,0x3e4*,0x3e8*,0x3ec*} | Steam {0x4*,0x8*,0xc*,0x3e4*,0x3e8*,0x3ec*}
imm sets (* = in both, 6 common): GOG {0x4*,0xa*,0x10*,0x20*,0x200*,0xffff*} | Steam {0x4*,0xa*,0x10*,0x20*,0x200*,0xffff*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0072e030: 16-byte aligned: YES
  0072e030  55             PUSH EBP
  0072e031  8B EC          MOV EBP,ESP
  0072e033  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0072e030
string literals (0):
direct callees (1):
  0x0072E730  (unnamed)
struct offsets touched (top 6):
  +0xC [dec 12]  x20  via ECX,EDX
  +0x3EC [dec 1004]  x20  via EAX,EDX
  +0x3E4 [dec 996]  x10  via ECX
  +0x3E8 [dec 1000]  x10  via EDX
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0072e030 FUN_0072e030 size=1743 ret=4 callers=1 callees=1

undefined4 __thiscall FUN_0072e030(int param_1,int param_2)

{
  undefined4 uVar1;
  uint uVar2;
  undefined4 local_18;
  int local_14;
  undefined4 local_8;
  
  switch(*(undefined4 *)(param_2 + 4)) {
  case 0x200:
    if (*(int *)(param_1 + 0x3ec) != 0) {
      *(undefined4 *)(param_1 + 0x3ec) = 0;
      return 1;
    }
    *(int *)(param_1 + 0x3e4) = (int)(short)*(undefined4 *)(param_2 + 0xc);
    *(int *)(param_1 + 0x3e8 [dec 1000]) = (int)(short)((uint)*(undefined4 *)(param_2 + 0xc) >> 0x10);
    return 1;
  case 0x201:
    if (*(int *)(param_1 + 0x3ec) == 0) {
      uVar1 = *(undefined4 *)(param_2 + 0xc);
      FUN_0072e730(DAT_0099c888,DAT_0099c8a8,(int)(short)*(undefined4 *)(param_2 + 0xc));
      FUN_0072e730(DAT_0099c888,DAT_0099c8ac,(int)(short)((uint)uVar1 >> 0x10));
      *(undefined4 *)(param_1 + 0x3e4) = 0xffffffff;
      *(undefined4 *)(param_1 + 0x3e8 [dec 1000]) = 0xffffffff;
    }
    else {
      *(undefined4 *)(param_1 + 0x3ec) = 0;
    }
    local_8 = DAT_0099c888;
    local_14 = 1;
    local_18 = DAT_0099c89c;
    break;
  case 0x202:
    if (*(int *)(param_1 + 0x3ec) == 0) {
      uVar1 = *(undefined4 *)(param_2 + 0xc);
      FUN_0072e730(DAT_0099c888,DAT_0099c8a8,(int)(short)*(undefined4 *)(param_2 + 0xc));
      FUN_0072e730(DAT_0099c888,DAT_0099c8ac,(int)(short)((uint)uVar1 >> 0x10));
      *(undefined4 *)(param_1 + 0x3e4) = 0xffffffff;
      *(undefined4 *)(param_1 + 0x3e8 [dec 1000]) = 0xffffffff;
    }
    else {
      *(undefined4 *)(param_1 + 0x3ec) = 0;
    }
    local_8 = DAT_0099c888;
    local_14 = 0;
    local_18 = DAT_0099c89c;
    break;
  case 0x203:
    if (*(int *)(param_1 + 0x3ec) == 0) {
      uVar1 = *(undefined4 *)(param_2 + 0xc);
      FUN_0072e730(DAT_0099c888,DAT_0099c8a8,(int)(short)*(undefined4 *)(param_2 + 0xc));
      FUN_0072e730(DAT_0099c888,DAT_0099c8ac,(int)(short)((uint)uVar1 >> 0x10));
      *(undefined4 *)(param_1 + 0x3e4) = 0xffffffff;
      *(undefined4 *)(param_1 + 0x3e8 [dec 1000]) = 0xffffffff;
    }
    else {
      *(undefined4 *)(param_1 + 0x3ec) = 0;
    }
    local_8 = DAT_0099c888;
    local_14 = -1;
    local_18 = DAT_0099c89c;
    break;
  case 0x204:
    if (*(int *)(param_1 + 0x3ec) == 0) {
      uVar1 = *(undefined4 *)(param_2 + 0xc);
      FUN_0072e730(DAT_0099c888,DAT_0099c8a8,(int)(short)*(undefined4 *)(param_2 + 0xc));
      FUN_0072e730(DAT_0099c888,DAT_0099c8ac,(int)(short)((uint)uVar1 >> 0x10));
      *(undefined4 *)(param_1 + 0x3e4) = 0xffffffff;
      *(undefined4 *)(param_1 + 0x3e8 [dec 1000]) = 0xffffffff;
    }
    else {
      *(undefined4 *)(param_1 + 0x3ec) = 0;
    }
    local_8 = DAT_0099c888;
    local_14 = 1;
    local_18 = DAT_0099c8a0;
    break;
  case 0x205:
/* ... 80 more lines truncated by --max-lines */
```

### decoy: 0x0072e730 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0072e730: 16-byte aligned: YES
  0072e730  55             PUSH EBP
  0072e731  8B EC          MOV EBP,ESP
  0072e733  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0072e730
string literals (0):
direct callees (3):
  0x00919723  operator new
  0x0091C630  (unnamed)
  0x0054AA10  CExoLinkedList::AddHead
struct offsets touched (top 7):
  +0xC [dec 12]  x6  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x10 [dec 16]  x1  via EAX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
  +0x1C [dec 28]  x1  via EAX
```

### Static facts (script output, report-only)
- size 1743 B, RET 4
- callers 1: 0x0072fbd0
- callees: 0x0072e730 (CALL)
- twins: none

## Card B17-6-18

GOG function: CResRef::operator==_3  gog 0x0061a570  callconv=(blank)  param_size_bytes=4  gog_size=113  gog_retn=4
Candidate Steam address: 0x00710c20 (size 113, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 9 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 9 (callers: ? 0x765940, GetEquipmentForcePowers_2 0x778ba0, SetEnvMap 0x7f03f0)
- best decoy (neighbour/other candidate) scores: 0x00710930:STR0/CALLEE5/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061a4c0 CResRef::operator==_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0061a5f0 CResRef::operator=  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00928a62 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (9 of 40): the card's A-CALLEE credit may come from these, not from callees
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

## Card B17-6-19

GOG function: CSWGuiOptionsGraphics::Constructor  gog 0x00569ef0  callconv=(blank)  param_size_bytes=8  gog_size=3078  gog_retn=8
Candidate Steam address: 0x007d5580 (size 884, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.15 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007d52e0:STR0/CALLEE0/SIG0.276/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00569e80 CSWGuiOptionsFeedback::OnEnableToolTips  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0056ab00 CSWGuiSlider::Destructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (22), mapped to Steam through trusted rows only:
  0x00512a40 CSWGuiButton::Constructor -> Steam ?  in candidate callees: n/a
  0x005253c0 Global::GetLocalizedCustomString -> Steam ?  in candidate callees: n/a
  0x005324a0 Global::AspyrAddButtonIcon -> Steam ?  in candidate callees: n/a
  0x00567cf0 Global::ExpandButtonToFitText -> Steam ?  in candidate callees: n/a
  0x0056b940 CSWGuiOptionsGraphics::SetUpOptions -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x0061a190 CResRef::Constructor_4 -> Steam ?  in candidate callees: n/a
  ... 14 more GOG callees
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (18): 'LBL_BAR4'; 'LBL_BAR3'; 'LBL_BAR2'; 'LBL_BAR1' ...
disp sets (* = in both, 3 common): GOG {0x4*,0x8,0x48*,0x98,0xc4,0x288,0x374,0x8d5,0x8e0,0x950,0xb20,0xcf0,0xec0,0x1188,...(+4)} | Steam {0x4*,0x1c,0x48*,0x1aac,0x214c*}
imm sets (* = in both, 11 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb,0xc,0xd,0xe,0xf,...(+58)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0x14*,0x28,0x74,0x200,0x1a5c*,...(+2)}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x007d5580: 16-byte aligned: YES
  007d5580  55             PUSH EBP
  007d5581  8B EC          MOV EBP,ESP
  007d5583  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007d5580
string literals (9):
  0x00986C98  at 007d568f  'Disable Vertex Buffer Objects'
  0x00986C84  at 007d56a0  'Graphics Options'
  0x00986B98  at 007d56b1  '.\\\\swkotor2.ini'
  0x00986C98  at 007d5732  'Disable Vertex Buffer Objects'
  0x00986C84  at 007d5743  'Graphics Options'
  0x00986B98  at 007d5754  '.\\\\swkotor2.ini'
  0x00986C6C  at 007d5807  'Disable Write-Only VBO'
  0x00986C84  at 007d5818  'Graphics Options'
  0x00986B98  at 007d5829  '.\\\\swkotor2.ini'
direct callees (15):
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x007D5AD0  (unnamed)
  0x004109C0  CSWGuiManager::PopModalPanel
  0x00736240  (unnamed)
  0x00733540  CExoString::Constructor
  0x00733570  CExoString::CExoString_char
  0x00736320  (unnamed)
  0x00733780  CExoString::~CExoString
  0x0091CF18  (unnamed)
  0x004791D0  Global::AurDisableVertexBufferObjects
  0x00409750  ChangeDisplayMode/RecreateWindow
  0x004791E0  Global::AurDisableVertexBufferObjectsWriteOnly
  0x004791F0  Global::AurEnableVertexBufferObjectsWriteOnly
  0x007362C0  (unnamed)
  0x0040EAA0  CSWGuiPanel::HandleInputEvent
struct offsets touched (top 5):
  +0x1C [dec 28]  x4  via EAX,ECX,EDX
  +0x48 [dec 72]  x4  via EAX,ECX
  +0x214C [dec 8524]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via EDX
  +0x1AAC [dec 6828]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007d5580 FUN_007d5580 size=884 ret=8 callers=0 callees=15

void __thiscall FUN_007d5580(int param_1,undefined4 param_2,int param_3)

{
  undefined4 uVar1;
  int local_7c;
  int local_78;
  int local_74;
  undefined1 local_70 [8];
  undefined1 local_68 [8];
  undefined1 local_60 [8];
  int local_58;
  undefined1 local_54 [8];
  undefined1 local_4c [8];
  undefined1 local_44 [8];
  int local_3c;
  undefined1 local_38 [8];
  undefined1 local_30 [8];
  undefined1 local_28 [8];
  int local_20;
  int local_18[2 = byte +0x8];
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0096a5c8;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (param_3 != 0) {
    switch(param_2) {
    case 0x28:
    case 0x2e:
      FUN_004122a0(0);
      FUN_004109c0();
      *(uint *)(param_1 + 0x48) = *(uint *)(param_1 + 0x48) & 0xffffff7f;
      *(uint *)(param_1 + 0x48) = *(uint *)(param_1 + 0x48) & 0xfffffcff | 0x200;
      if (*(int *)(param_1 + 0x214c) != 0) {
        FUN_00736240();
        local_8 = 0;
        FUN_00733540();
        local_8._0_1_ = 1;
        FUN_00733570("Disable Vertex Buffer Objects");
        local_8._0_1_ = 2;
        FUN_00733570("Graphics Options");
        local_8._0_1_ = 3;
        FUN_00733570(".\\swkotor2.ini");
        local_8._0_1_ = 4;
        local_20 = FUN_00736320(local_18,local_38,local_30,local_28);
        local_8._0_1_ = 3;
        FUN_00733780();
        local_8._0_1_ = 2;
        FUN_00733780();
        local_8 = CONCAT31(local_8._1_3_,1);
        FUN_00733780();
        if (local_20 != 0) {
          if (local_18[0] == 0) {
            local_74 = 0;
          }
          else {
            local_74 = FUN_0091cf18(local_18[0]);
          }
          if (local_74 == 1) {
            FUN_004791d0();
          }
        }
        FUN_00733570("Disable Vertex Buffer Objects");
        local_8._0_1_ = 5;
        FUN_00733570("Graphics Options");
        local_8._0_1_ = 6;
        FUN_00733570(".\\swkotor2.ini");
        local_8._0_1_ = 7;
        local_3c = FUN_00736320(local_18,local_54,local_4c,local_44);
        local_8._0_1_ = 6;
        FUN_00733780();
        local_8._0_1_ = 5;
        FUN_00733780();
        local_8 = CONCAT31(local_8._1_3_,1);
        FUN_00733780();
        if (local_3c != 0) {
/* ... 59 more lines truncated by --max-lines */
```

### decoy: 0x007d52e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007d52e0: 16-byte aligned: YES
  007d52e0  55             PUSH EBP
  007d52e1  8B EC          MOV EBP,ESP
  007d52e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007d52e0
string literals (0):
direct callees (8):
  0x00919BCC  `eh vector destructor iterator` (CRT)
  0x0041A3D0  CSWGuiButton::Destructor
  0x00414C60  CSWGuiBorder::Destructor_2
  0x0041D190  (unnamed)
  0x00417290  (unnamed)
  0x00418640  CSWGuiControl::Destructor
  0x0041AC10  (unnamed)
  0x0040E860  CSWGuiPanel::Destructor
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 884 B, RET 8
- callees: 0x004122a0 CSWGuiManager::PlayGuiSound (CALL), 0x007d5ad0 (CALL), 0x004109c0 CSWGuiManager::PopModalPanel (CALL), 0x00736240 (CALL), 0x00733540 CExoString::DefaultConstructor (CALL), 0x00733570 CExoString::CStrConstructor (CALL), ...
- indirect: 0x007d57e7 slot 0x4
- globals: 0x00a1b4a0 READ, 0x00a1b4a0 READ
- twins: none

## Card B17-6-20

GOG function: CExoRawInputInternal::GetMouseState  gog 0x00619a00  callconv=(blank)  param_size_bytes=4  gog_size=253  gog_retn=4
Candidate Steam address: 0x00732e00 (size 253, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00732c40:STR0/CALLEE0/SIG0.631/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00619840 CExoRawInputInternal::GetKeyboardBuffer  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00619b00 CExoRawInputInternal::KeyboardModeOn  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00617c70 CExoRawInputInternal::InitializeDirectInputMouse -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x1c*,0x24*,0x28*} | Steam {0x4*,0x1c*,0x24*,0x28*}
imm sets (* = in both, 8 common): GOG {0x4*,0xc*,0x10*,0x8000000a*,0x8007000c*,0x80070015*,0x8007001e*,0x80070057*} | Steam {0x4*,0xc*,0x10*,0x8000000a*,0x8007000c*,0x80070015*,0x8007001e*,0x80070057*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00732e00: 16-byte aligned: YES
  00732e00  55             PUSH EBP
  00732e01  8B EC          MOV EBP,ESP
  00732e03  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00732e00
string literals (0):
direct callees (1):
  0x00731070  (unnamed)
struct offsets touched (top 4):
  +0x28 [dec 40]  x6  via EAX,ECX,EDX
  +0x4 [dec 4]  x1  via EDX
  +0x1C [dec 28]  x1  via EAX
  +0x24 [dec 36]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00732e00 FUN_00732e00 size=253 ret=4 callers=1 callees=1

undefined4 __thiscall FUN_00732e00(int param_1,undefined4 param_2)

{
  undefined4 uVar1;
  int iVar2;
  int local_8;
  
  if (*(int *)(param_1 + 0x28) == 0) {
    FUN_00731070();
  }
  if (*(int *)(param_1 + 0x28) == 0) {
    uVar1 = 0;
  }
  else if (*(int *)(param_1 + 4) == 0) {
    uVar1 = 0;
  }
  else {
    local_8 = -0x7ff8ffe2;
    while (local_8 == -0x7ff8ffe2) {
      iVar2 = (**(code **)(**(int **)(param_1 + 0x28) + 0x24))
                        (*(undefined4 *)(param_1 + 0x28),0x10,param_2);
      if (iVar2 == 0) {
        return 1;
      }
      if (iVar2 < -0x7ff8ffea) {
        if (iVar2 == -0x7ff8ffeb) {
          return 0;
        }
        if (iVar2 == -0x7ffffff6) {
          return 0;
        }
        if (iVar2 != -0x7ff8fff4) {
          return 0;
        }
      }
      else if (iVar2 != -0x7ff8ffe2) {
        if (iVar2 != -0x7ff8ffa9) {
          return 0;
        }
        return 0;
      }
      iVar2 = (**(code **)(**(int **)(param_1 + 0x28) + 0x1c))(*(undefined4 *)(param_1 + 0x28));
      if (iVar2 != 0) {
        return 0;
      }
      local_8 = 0;
    }
    uVar1 = 1;
  }
  return uVar1;
}
```

### decoy: 0x00732c40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00732c40: 16-byte aligned: YES
  00732c40  55             PUSH EBP
  00732c41  8B EC          MOV EBP,ESP
  00732c43  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00732c40
string literals (0):
direct callees (3):
  0x009196FD  operator delete
  0x00919723  operator new
  0x0091C630  (unnamed)
struct offsets touched (top 5):
  +0x24 [dec 36]  x7  via EAX,ECX,EDX
  +0x4 [dec 4]  x3  via EAX,ECX
  +0x1C [dec 28]  x2  via ECX
  +0x8 [dec 8]  x1  via EDX
  +0x28 [dec 40]  x1  via EAX
```

### Static facts (script output, report-only)
- size 253 B, RET 4
- callers 1: 0x0072f110
- callees: 0x00731070 (CALL)
- indirect: 0x00732e66 slot 0x24, 0x00732ec3 slot 0x1c
- twins: none

## Card B17-6-21

GOG function: CSWGuiNameChargen::Destructor  gog 0x00604f00  callconv=(blank)  param_size_bytes=0  gog_size=317  gog_retn=0
Candidate Steam address: 0x00919040 (size 319, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.07 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00918f00:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00604ed0 CSWGuiNameChargen::scalar  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00605040 CSWGuiNameChargen::OnPanelAdded  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x004ce500 CSWGuiControl::Destructor -> Steam 0x00418640  in candidate callees: no
  0x004ce630 CSWGuiButton::Destructor -> Steam 0x0041a3d0  in candidate callees: no
  0x005d7d00 CSWGuiEditbox::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x008115a0 CSWNameGen::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x0090cce0 CSWGuiPanel::Destructor -> Steam 0x0040e860  in candidate callees: no
  0x00913030 CSWGuiBorder::Destructor_2 -> Steam 0x00414c60  in candidate callees: no
  0x00915660 CSWGuiText::Destructor_2 -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x6c,0x9d8}
imm sets (* = in both, 3 common): GOG {0x2*,0x3,0x4*,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0x10,0x60,0x70,0xd8,...(+7)} | Steam {0x2*,0x4*,0x30,0x240*,0x328}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00919040: 16-byte aligned: YES
  00919040  55             PUSH EBP
  00919041  8B EC          MOV EBP,ESP
  00919043  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00919040
string literals (0):
direct callees (10):
  0x00733540  CExoString::Constructor
  0x004215E0  (unnamed)
  0x0073EEE0  (unnamed)
  0x0040F3C0  CSWGuiPanel::OnPanelAdded
  0x00916600  (unnamed)
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x00919F10  _strlen
  0x00919300  (unnamed)
  0x00417DD0  (unnamed)
struct offsets touched (top 2):
  +0x6C [dec 108]  x2  via ECX
  +0x9D8 [dec 2520]  x2  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00919040 FUN_00919040 size=319 ret=0 callers=0 callees=10

void __fastcall FUN_00919040(int param_1)

{
  char cVar1;
  undefined4 uVar2;
  size_t local_2c;
  undefined1 local_28 [8];
  undefined1 local_20 [8];
  char *local_18 [2];
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00982a38;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  FUN_004215e0();
  cVar1 = FUN_0073eee0();
  if (cVar1 == '\0') {
    FUN_0040f3c0();
  }
  if (*(char *)(param_1 + 0x9d8) == '\x01') {
    uVar2 = FUN_00916600(local_20);
    local_8._0_1_ = 1;
    FUN_007337c0(uVar2);
    local_8 = (uint)local_8._1_3_ << 8;
    FUN_00733780();
  }
  else if (*(char *)(param_1 + 0x9d8) == '\x02') {
    uVar2 = FUN_00916600(local_28);
    local_8._0_1_ = 2;
    FUN_007337c0(uVar2);
    local_8 = (uint)local_8._1_3_ << 8;
    FUN_00733780();
  }
  if (local_18[0] == (char *)0x0) {
    local_2c = 0;
  }
  else {
    local_2c = _strlen [GHIDRA-AUTO-LABEL: unreliable](local_18[0]);
  }
  if (local_2c == 0) {
    FUN_00919300();
  }
  else {
    FUN_00417dd0(local_18);
  }
  local_8 = 0xffffffff;
  FUN_00733780();
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x00918f00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00918f00: 16-byte aligned: YES
  00918f00  55             PUSH EBP
  00918f01  8B EC          MOV EBP,ESP
  00918f03  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00918f00
string literals (0):
direct callees (8):
  0x00942280  (unnamed)
  0x00733780  CExoString::~CExoString
  0x0041A3D0  CSWGuiButton::Destructor
  0x00417290  (unnamed)
  0x00414C60  CSWGuiBorder::Destructor_2
  0x00418640  CSWGuiControl::Destructor
  0x00421060  (unnamed)
  0x0040E860  CSWGuiPanel::Destructor
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 319 B, RET 0
- callees: 0x00733540 CExoString::DefaultConstructor (CALL), 0x004215e0 (CALL), 0x0073eee0 (CALL), 0x0040f3c0 CSWGuiPanel::OnPanelAdded (CALL), 0x00916600 (CALL), 0x007337c0 CExoString::operator= (CALL), ...
- twins: none

## Card B17-6-22

GOG function: MdlNodeTriMesh::ProcessVerts  gog 0x0083e200  callconv=(blank)  param_size_bytes=0  gog_size=2857  gog_retn=0
Candidate Steam address: 0x0044b8e0 (size 1167, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.86) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.37 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0044bd70:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0083dd70 MdlNodeTriMesh::InternalGenVertices  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0083ed70 MdlNodeTriMesh::BuildVertexIndices  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x00413280 Vector::operator+ -> Steam ?  in candidate callees: n/a
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x00419020 Vector::operator* -> Steam ?  in candidate callees: n/a
  0x0080c0a0 Global::float_magnitude_Vector_const_ref -> Steam ?  in candidate callees: n/a
  0x0080c0f0 Vector::Normalize -> Steam ?  in candidate callees: n/a
  0x0081b9c0 unnamed -> Steam ?  in candidate callees: n/a
  0x0083ed30 unnamed -> Steam ?  in candidate callees: n/a
  0x00924490 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 12 common): GOG {0x4*,0x8,0xc*,0x10*,0x14,0x18*,0x1c*,0x20,0x24*,0x28*,0x2c,0x48*,0x4c*,0x58*,...(+9)} | Steam {0x4*,0xc*,0x10*,0x18*,0x1a,0x1c*,0x1e,0x24*,0x28*,0x30,0x34,0x3c,0x40,0x48*,...(+7)}
imm sets (* = in both, 5 common): GOG {0x3*,0x4*,0x5*,0x8,0xc*,0x14*,0x41,0x44,0x54,0x80,0x194} | Steam {0x3*,0x4*,0x5*,0xc*,0x14*,0x58,0x60,0x88}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0044b8e0: 16-byte aligned: YES
  0044b8e0  55             PUSH EBP
  0044b8e1  8B EC          MOV EBP,ESP
  0044b8e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0044b8e0
string literals (0):
direct callees (7):
  0x00919723  operator new
  0x004CD700  (unnamed)
  0x004CDA60  (unnamed)
  0x009196FD  operator delete
  0x004491C0  CExoPtrList::Add (doubling)
  0x004543C0  (unnamed)
  0x004517A0  (unnamed)
struct offsets touched (top 10):
  +0x134 [dec 308]  x24  via EAX,ECX,EDX
  +0x58 [dec 88]  x8  via EAX,ECX,EDX
  +0x48 [dec 72]  x7  via EAX,ECX,EDX
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x10 [dec 16]  x2  via EAX,ECX
  +0x1A [dec 26]  x2  via EAX,ECX
  +0x1C [dec 28]  x2  via EDX
  +0x64 [dec 100]  x2  via ECX
  +0xC [dec 12]  x1  via EAX
  +0x18 [dec 24]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0044b8e0 FUN_0044b8e0 size=1167 ret=0 callers=1 callees=7

void __fastcall FUN_0044b8e0(int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  bool bVar4;
  char cVar5;
  uint uVar6;
  int iVar7;
  int iVar8;
  undefined4 local_98;
  int local_94;
  int local_90;
  int local_8c;
  int local_88;
  int local_28;
  int local_24;
  int local_1c;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009493cb;
  local_10 = ExceptionList;
  uVar6 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_14 = 0;
  do {
    if (*(int *)(*(int *)(param_1 + 0x134) + 0x4c) <= local_14) {
      iVar8 = FUN_004543c0(uVar6);
      if (iVar8 == 0) {
        local_14 = *(int *)(param_1 + 0x5c);
        while (iVar8 = local_14 + -1, local_14 != 0) {
          local_14 = iVar8;
          if (((*(short *)(iVar8 * 0x20 + *(int *)(param_1 + 0x58) + 0x1a) ==
                *(short *)(*(int *)(param_1 + 0x58) + 0x1c + iVar8 * 0x20)) ||
              (*(short *)(iVar8 * 0x20 + *(int *)(param_1 + 0x58) + 0x1a) ==
               *(short *)(*(int *)(param_1 + 0x58) + 0x1e + iVar8 * 0x20))) ||
             (*(short *)(iVar8 * 0x20 + *(int *)(param_1 + 0x58) + 0x1c) ==
              *(short *)(*(int *)(param_1 + 0x58) + 0x1e + iVar8 * 0x20))) {
            FUN_004517a0(iVar8);
          }
        }
      }
      ExceptionList = local_10;
      return;
    }
    for (local_24 = 0; local_24 < 3; local_24 = local_24 + 1) {
      iVar8 = *(int *)(*(int *)(local_14 * 0x14 + *(int *)(*(int *)(param_1 + 0x134) + 0x48)) +
                      local_24 * 4);
      iVar1 = **(int **)(param_1 + 0x134);
      if (*(int *)(*(int *)(param_1 + 0x134) + 0x10) == 0) {
        local_88 = 0;
      }
      else {
        local_88 = *(int *)(*(int *)(local_14 * 0x14 + *(int *)(*(int *)(param_1 + 0x134) + 0x48) +
                                    4) + local_24 * 4) * 0xc +
                   *(int *)(*(int *)(param_1 + 0x134) + 0xc);
      }
      if (*(int *)(*(int *)(param_1 + 0x134) + 0x1c) == 0) {
        local_8c = 0;
      }
      else {
        local_8c = *(int *)(*(int *)(local_14 * 0x14 + *(int *)(*(int *)(param_1 + 0x134) + 0x48) +
                                    4) + 0xc + local_24 * 4) * 0xc +
                   *(int *)(*(int *)(param_1 + 0x134) + 0x18);
      }
      if (*(int *)(*(int *)(param_1 + 0x134) + 0x28) == 0) {
        local_90 = 0;
      }
      else {
        local_90 = *(int *)(*(int *)(local_14 * 0x14 + *(int *)(*(int *)(param_1 + 0x134) + 0x48) +
                                    4) + 0x18 + local_24 * 4) * 0xc +
                   *(int *)(*(int *)(param_1 + 0x134) + 0x24);
      }
      if (*(int *)(*(int *)(param_1 + 0x134) + 0x34) == 0) {
/* ... 46 more lines truncated by --max-lines */
```

### decoy: 0x0044bd70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0044bd70: 16-byte aligned: YES
  0044bd70  55             PUSH EBP
  0044bd71  8B EC          MOV EBP,ESP
  0044bd73  81 EC 94 01 00 00 SUB ESP,0x194
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0044bd70
string literals (0):
direct callees (8):
  0x00429330  (unnamed)
  0x0091C860  _ftol2_sse
  0x0044C900  (unnamed)
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044C8A0  Vector::operator+ (Vector3 add)
  0x005142F0  (unnamed)
  0x00514340  (unnamed)
  0x00436200  (unnamed)
struct offsets touched (top 10):
  +0x134 [dec 308]  x38  via EAX,ECX,EDX
  +0x4 [dec 4]  x28  via EAX,ECX,EDX
  +0x48 [dec 72]  x25  via EAX,ECX,EDX
  +0x8 [dec 8]  x22  via EAX,ECX,EDX
  +0x4C [dec 76]  x3  via EAX,ECX
  +0x64 [dec 100]  x3  via EAX,ECX,EDX
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0x6C [dec 108]  x3  via EAX,ECX,EDX
  +0x70 [dec 112]  x3  via EAX,ECX,EDX
  +0x74 [dec 116]  x3  via EAX,ECX,EDX
```

### Static facts (script output, report-only)
- size 1167 B, RET 0
- callers 1: 0x00455e50
- callees: 0x00919723 operator new (CALL), 0x004cd700 (CALL), 0x004cda60 (CALL), 0x009196fd operator delete (CALL), 0x004491c0 (CALL), 0x004543c0 (CALL), ...
- twins: none
