# Review cards B23 chunk 3

Answer each card with one TSV row in `steam_db_batches/B23/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B23-3-01

GOG function: CAuroraTexture::GetTextureDataSize  gog 0x0090a590  callconv=(blank)  param_size_bytes=0  gog_size=51  gog_retn=0
Candidate Steam address: 0x0093f830 (size 51, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0093f870:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0090a550 CAuroraProcessedTexture::GetProcessedTextureDataSize -> 0x0093feb0  gate_class=REVIEW
- GOG-order neighbour after:  0x0090a5d0 CAuroraProcessedTexture::Load -> 0x0093ffb0  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x005915c0 CResTPC::GetTPCTXISize -> Steam ?  in candidate callees: n/a
  0x0061abb0 CRes::GetDemands -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x0090a8f0 CAuroraProcessedTexture::GetPtrToResHelperInfo -> Steam 0x009402d0  in candidate callers: no
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

### decoy: 0x0093f870 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0093f870: 16-byte aligned: YES
  0093f870  55             PUSH EBP
  0093f871  8B EC          MOV EBP,ESP
  0093f873  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0093f870
string literals (0):
direct callees (2):
  0x00711250  (unnamed)
  0x00945EA0  (unnamed)
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

## Card B23-3-02

GOG function: CAuroraModel::GetModelDataPtr  gog 0x0090bab0  callconv=(blank)  param_size_bytes=0  gog_size=51  gog_retn=0
Candidate Steam address: 0x009410e0 (size 164, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00941090:STR0/CALLEE0/SIG0.25/SRV0/VT0
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
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x4,0x8*}
imm sets (* = in both, 0 common): GOG {} | Steam {0xc,0x10}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x009410e0: 16-byte aligned: YES
  009410e0  55             PUSH EBP
  009410e1  8B EC          MOV EBP,ESP
  009410e3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x009410e0
string literals (0):
direct callees (3):
  0x00710AD0  (unnamed)
  0x007111C0  CRes::CancelRequest
  0x00713010  CExoResMan::ReleaseResObject
struct offsets touched (top 2):
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 009410e0 FUN_009410e0 size=164 ret=0 callers=3 callees=3

void __fastcall FUN_009410e0(undefined4 *param_1)

{
  int iVar1;
  
  *param_1 = CResHelper<CResMDX,3008>::vftable;
  if ((param_1[2 = byte +0x8] != 0) && (iVar1 = FUN_00710ad0(&DAT_00986925), iVar1 != 0)) {
    if (param_1[1 = byte +0x4] != 0) {
      FUN_007111c0();
      param_1[1 = byte +0x4] = 0;
    }
    iVar1 = FUN_00713010(param_1[2 = byte +0x8]);
    if (iVar1 == 0) {
      if ((undefined4 *)param_1[2 = byte +0x8] != (undefined4 *)0x0) {
        (*(code *)**(undefined4 **)param_1[2 = byte +0x8])(1);
      }
      param_1[2 = byte +0x8] = 0;
    }
  }
  return;
}
```

### decoy: 0x00941090 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00941090: 16-byte aligned: YES
  00941090  55             PUSH EBP
  00941091  8B EC          MOV EBP,ESP
  00941093  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00941090
string literals (0):
direct callees (2):
  0x007107F0  (unnamed)
  0x00710EB0  CResRef::operator=_4
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
```

### Static facts (script output, report-only)
- size 164 B, RET 0
- callers 3: 0x00941020, 0x00941350, 0x00982f80
- callees: 0x00710ad0 (CALL), 0x007111c0 CRes::CancelRequest (CALL), 0x00713010 CExoResMan::ReleaseResObject (CALL)
- indirect: 0x00941168 slot 0x0
- globals: 0x00a1b490 READ
- twins: 0x00940ce0 (distinguishing: 0x009410ec: MOV dword ptr [EAX],0x9b95e0)

### Live evidence (REA, report-only)
- (no REA rows for 0x009410e0)

## Card B23-3-03

GOG function: CSWGuiExtent::Union  gog 0x0090ca10  callconv=(blank)  param_size_bytes=12  gog_size=285  gog_retn=0
Candidate Steam address: 0x0040e480 (size 285, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0040e360:STR0/CALLEE0/SIG1/SRV0/VT0
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
prologue for 0x0040e480: 16-byte aligned: YES
  0040e480  55             PUSH EBP
  0040e481  8B EC          MOV EBP,ESP
  0040e483  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0040e480
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x9  via EAX,ECX,EDX
  +0x8 [dec 8]  x5  via EAX
  +0xC [dec 12]  x5  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0040e480 FUN_0040e480 size=285 ret=0 callers=1 callees=0

int * FUN_0040e480(int *param_1,int *param_2,int *param_3)

{
  int local_2c;
  int local_28;
  int local_24;
  int local_20;
  
  if (*param_2 < *param_3) {
    local_20 = *param_2;
  }
  else {
    local_20 = *param_3;
  }
  if (param_2[1 = byte +0x4] < param_3[1 = byte +0x4]) {
    local_24 = param_2[1 = byte +0x4];
  }
  else {
    local_24 = param_3[1 = byte +0x4];
  }
  if (*param_3 + param_3[2 = byte +0x8] < *param_2 + param_2[2 = byte +0x8]) {
    local_28 = *param_2 + param_2[2 = byte +0x8];
  }
  else {
    local_28 = *param_3 + param_3[2 = byte +0x8];
  }
  if (param_3[1 = byte +0x4] + param_3[3 = byte +0xc] < param_2[1 = byte +0x4] + param_2[3 = byte +0xc]) {
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

### decoy: 0x0040e360 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0040e360: 16-byte aligned: YES
  0040e360  55             PUSH EBP
  0040e361  8B EC          MOV EBP,ESP
  0040e363  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0040e360
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x9  via EAX,ECX,EDX
  +0x8 [dec 8]  x5  via EAX
  +0xC [dec 12]  x5  via EAX,ECX
```

### Static facts (script output, report-only)
- size 285 B, RET 0
- callers 1: 0x008690a0
- twins: none

### Live evidence (REA, report-only)
- xref | no live run (static only) | probable | rea_facts 0x0040e480: static only
- xref | no live-only facts | probable | rea_facts 0x0040e480: checked

## Card B23-3-04

GOG function: CSWGuiExtent::Intersection  gog 0x0090c8f0  callconv=(blank)  param_size_bytes=12  gog_size=285  gog_retn=0
Candidate Steam address: 0x0040e360 (size 285, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (callers: SetExtent 0x8d0110, SetExtent 0x419ed0)
- best decoy (neighbour/other candidate) scores: 0x0040e480:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0090c850 CSWGuiExtent::Load  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0090ca10 CSWGuiExtent::Union -> 0x0040e480  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (2 of 10): the card's A-CALLEE credit may come from these, not from callees
  0x005c04e0 CSWUpgradeItemEntry::SetExtent -> Steam 0x008d0110  in candidate callers: yes
  0x00918170 CSWGuiButton::SetExtent -> Steam 0x00419ed0  in candidate callers: yes
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

## Card B23-3-05

GOG function: CAuroraModel::GetModelDataPtr  gog 0x0090bab0  callconv=(blank)  param_size_bytes=0  gog_size=51  gog_retn=0
Candidate Steam address: 0x00941490 (size 51, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
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
prologue for 0x00941490: 16-byte aligned: YES
  00941490  55             PUSH EBP
  00941491  8B EC          MOV EBP,ESP
  00941493  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00941490
string literals (0):
direct callees (2):
  0x00711250  (unnamed)
  0x006F3D90  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00941490 FUN_00941490 size=51 ret=0 callers=6 callees=2

undefined4 __fastcall FUN_00941490(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  if ((*(int *)(param_1 + 8) != 0) && (iVar1 = FUN_00711250(), 0 < iVar1)) {
    uVar2 = FUN_006f3d90();
    return uVar2;
  }
  return 0;
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
- size 51 B, RET 0
- callers 6: 0x0093b770, 0x0093bc00 ResourceProvider(name, ext, &size) (DAT_00A3082C), 0x0093c470, 0x0093d160, 0x0093d670, ...
- callees: 0x00711250 (CALL), 0x006f3d90 (CALL)
- twins: none

### Live evidence (REA, report-only)
- xref | no live run (static only) | probable | rea_facts 0x00941490: static only
- xref | no live-only facts | probable | rea_facts 0x00941490: checked

## Card B23-3-06

GOG function: CAuroraTexture::GetTextureDataSize  gog 0x0090a590  callconv=(blank)  param_size_bytes=0  gog_size=51  gog_retn=0
Candidate Steam address: 0x0093f8b0 (size 187, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0093f870:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0090a550 CAuroraProcessedTexture::GetProcessedTextureDataSize -> 0x0093feb0  gate_class=REVIEW
- GOG-order neighbour after:  0x0090a5d0 CAuroraProcessedTexture::Load -> 0x0093ffb0  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x005915c0 CResTPC::GetTPCTXISize -> Steam ?  in candidate callees: n/a
  0x0061abb0 CRes::GetDemands -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x0090a8f0 CAuroraProcessedTexture::GetPtrToResHelperInfo -> Steam 0x009402d0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x2,0x4,0x8*}
imm sets (* = in both, 0 common): GOG {} | Steam {0x3,0x8,0x18,0x20}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0093f8b0: 16-byte aligned: YES
  0093f8b0  55             PUSH EBP
  0093f8b1  8B EC          MOV EBP,ESP
  0093f8b3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0093f8b0
string literals (0):
direct callees (4):
  0x007111E0  CRes::Demand
  0x009453A0  (unnamed)
  0x007112B0  CRes::Release
  0x009453F0  (unnamed)
struct offsets touched (top 2):
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0x2 [dec 2]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0093f8b0 FUN_0093f8b0 size=187 ret=0 callers=1 callees=4

undefined4 __fastcall FUN_0093f8b0(undefined4 param_1)

{
  undefined2 *puVar1;
  int iVar2;
  undefined4 uVar3;
  byte local_15;
  int local_14;
  int local_10;
  int local_c[2 = byte +0x8];
  
  iVar2 = FUN_007111e0();
  if (iVar2 == 0) {
    uVar3 = 0;
  }
  else {
    FUN_009453a0(local_c,&local_10,&local_15);
    if (((local_15 == 8) || (local_15 == 0x18)) || (local_15 == 0x20)) {
      local_14 = FUN_009453f0();
      if (local_14 == 0) {
        FUN_007112b0();
        uVar3 = 0;
      }
      else {
        puVar1 = (undefined2 *)(local_14 + 4 + local_c[0] * local_10 * ((int)(uint)local_15 >> 3));
        *puVar1 = 3;
        *(undefined4 *)(puVar1 + 0x1 [dec 1] {undefined2*: byte +0x2}) = param_1;
        uVar3 = 1;
      }
    }
    else {
      FUN_007112b0();
      uVar3 = 0;
    }
  }
  return uVar3;
}
```

### decoy: 0x0093f870 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0093f870: 16-byte aligned: YES
  0093f870  55             PUSH EBP
  0093f871  8B EC          MOV EBP,ESP
  0093f873  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0093f870
string literals (0):
direct callees (2):
  0x00711250  (unnamed)
  0x00945EA0  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
```

### Static facts (script output, report-only)
- size 187 B, RET 0
- callers 1: 0x0093df80
- callees: 0x007111e0 CRes::Demand (CALL), 0x009453a0 (CALL), 0x007112b0 CRes::Release (CALL), 0x009453f0 (CALL)
- twins: none

### Live evidence (REA, report-only)
- (no REA rows for 0x0093f8b0)
