# Review cards B17 chunk 3

Answer each card with one TSV row in `steam_db_batches/B17/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B17-3-01

GOG function: CSWMessage::WriteDWORD  gog 0x0054ba90  callconv=(blank)  param_size_bytes=8  gog_size=118  gog_retn=8
Candidate Steam address: 0x005e43b0 (size 118, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005e41b0:STR0/CALLEE2/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0054ba10 CSWMessage::WriteSHORT  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0054bb10 CSWMessage::WriteINT  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0054a8c0 CSWMessage::WriteUnsigned -> Steam ?  in candidate callees: n/a
  0x0054b6c0 CSWMessage::ExtendWriteBuffer -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 48): the card's A-CALLEE credit may come from these, not from callees
  0x00506050 CSWCMessage::SendPlayerToServer_LevelUp -> Steam 0x0087a770  in candidate callers: yes
  0x0071dac0 CSWSArea::PackAreaIntoMessage -> Steam 0x00528910  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0xc*,0x1c*} | Steam {0x4*,0x8*,0xc*,0x1c*}
imm sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x005e43b0: 16-byte aligned: YES
  005e43b0  55             PUSH EBP
  005e43b1  8B EC          MOV EBP,ESP
  005e43b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005e43b0
string literals (0):
direct callees (2):
  0x005E3FE0  (unnamed)
  0x005E31E0  (unnamed)
struct offsets touched (top 4):
  +0xC [dec 12]  x4  via EAX,ECX
  +0x1C [dec 28]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005e43b0 FUN_005e43b0 size=118 ret=8 callers=48 callees=2

/* WARNING: Removing unreachable block (ram,0x005e440d) */

void __thiscall FUN_005e43b0(int param_1,undefined4 param_2)

{
  if (*(uint *)(param_1 + 8) < *(int *)(param_1 + 0xc) + 4U) {
    FUN_005e3fe0(4);
  }
  *(undefined4 *)(*(int *)(param_1 + 4) + *(int *)(param_1 + 0xc)) = param_2;
  *(int *)(param_1 + 0xc) = *(int *)(param_1 + 0xc) + 4;
  *(int *)(param_1 + 0x1c) = *(int *)(param_1 + 0x1c) + 4;
  return;
}
```

### decoy: 0x005e41b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005e41b0: 16-byte aligned: YES
  005e41b0  55             PUSH EBP
  005e41b1  8B EC          MOV EBP,ESP
  005e41b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005e41b0
string literals (0):
direct callees (2):
  0x005E3FE0  (unnamed)
  0x005E31E0  (unnamed)
struct offsets touched (top 4):
  +0xC [dec 12]  x4  via EAX,ECX
  +0x1C [dec 28]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via EAX
```

### Static facts (script output, report-only)
- size 118 B, RET 8
- callers 48: 0x00528910 CSWSArea::PackAreaIntoMessage, 0x005e31e0, 0x0062a300, 0x00632220, 0x00633190, ...
- callees: 0x005e3fe0 (CALL), 0x005e31e0 (CALL)
- twins: none

## Card B17-3-02

GOG function: MdlNodeLight::InternalParseField  gog 0x008c7500  callconv=(blank)  param_size_bytes=4  gog_size=2293  gog_retn=4
Candidate Steam address: 0x00486aa0 (size 1295, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.80) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.13 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00486fb0:STR26/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008c6ff0 MdlNode::InternalParseField  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008c7e00 MdlNodeEmitter::InternalParseField -> 0x004878b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (12), mapped to Steam through trusted rows only:
  0x0081b7a0 Global::Parse -> Steam ?  in candidate callees: n/a
  0x008c5260 Global::AurMessageGetToken -> Steam ?  in candidate callees: n/a
  0x008c5350 Global::AurMessageParseFloat -> Steam ?  in candidate callees: n/a
  0x008c5370 Global::AurMessageParseIntOrBool -> Steam ?  in candidate callees: n/a
  0x008c5f90 Global::Parse_11 -> Steam ?  in candidate callees: n/a
  0x008c6720 Global::AddGenericControllerSingle -> Steam ?  in candidate callees: n/a
  0x008c6c50 Global::AddGenericController -> Steam ?  in candidate callees: n/a
  0x008c6e50 Global::AddBezierController -> Steam ?  in candidate callees: n/a
  ... 4 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (28): 'fadingLight'; 'flareradius'; 'flarecolorshifts'; 'flarepositions' ...
disp sets (* = in both, 0 common): GOG {} | Steam {0x8,0xc,0x28}
imm sets (* = in both, 7 common): GOG {0x4*,0x8*,0xc*,0x18*,0x38*,0x44*,0x4c,0x50,0x58,0x60,0x64*,0x6c,0x78,0x84,...(+8)} | Steam {0x4*,0x8*,0xc*,0x10,0x14,0x18*,0x1c,0x24,0x2c,0x38*,0x44*,0x64*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00486aa0: 16-byte aligned: YES
  00486aa0  55             PUSH EBP
  00486aa1  8B EC          MOV EBP,ESP
  00486aa3  83 EC 64       SUB ESP,0x64
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00486aa0
string literals (24):
  0x0098C6E4  at 00486add  'position'
  0x0098C6E4  at 00486af5  'position'
  0x0098C6D8  at 00486b1b  'orientation'
  0x0098C6D8  at 00486b33  'orientation'
  0x0098C6E4  at 00486b59  'position'
  0x0098C6E4  at 00486b81  'position'
  0x0098C6CC  at 00486bb9  'positionkey'
  0x0098C6CC  at 00486be6  'positionkey'
  0x0098C6B8  at 00486c1b  'positionbezierkey'
  0x0098C6B8  at 00486c48  'positionbezierkey'
  0x0098C6D8  at 00486c7b  'orientation'
  0x0098C6D8  at 00486ca3  'orientation'
direct callees (13):
  0x00484D10  Global::AurMessageGetToken
  0x009370C7  _stricmp
  0x00919F10  _strlen
  0x00484D90  (unnamed)
  0x00484FD0  (unnamed)
  0x004861D0  (unnamed)
  0x00486700  (unnamed)
  0x00486900  (unnamed)
  0x0091CF23  (unnamed)
  0x0091C7AD  (unnamed)
  0x00485D20  (unnamed)
  0x00473F40  vector<T*>::push_back (`ret 4`)
  0x009196EE  __security_check_cookie
struct offsets touched (top 3):
  +0xC [dec 12]  x4  via ECX,EDX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x28 [dec 40]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00486aa0 FUN_00486aa0 size=1295 ret=4 callers=6 callees=13

void __thiscall FUN_00486aa0(int param_1,char *param_2)

{
  char *pcVar1;
  int iVar2;
  size_t sVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  char *_Format [GHIDRA-AUTO-LABEL: unreliable];
  undefined4 uVar7;
  undefined4 *puVar8;
  char local_64 [68];
  uint local_20;
  int local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  int local_c;
  int local_8;
  
  local_20 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  local_c = 0;
  local_8 = 0;
  if (DAT_00a309bd == '\0') {
    pcVar1 = (char *)FUN_00484d10(param_2);
    iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("position",pcVar1);
    if (iVar2 == 0) {
      iVar2 = param_1 + 0x10;
      sVar3 = _strlen [GHIDRA-AUTO-LABEL: unreliable]("position");
      FUN_00484d90(param_2 + sVar3,iVar2);
    }
    pcVar1 = (char *)FUN_00484d10(param_2);
    iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("orientation",pcVar1);
    if (iVar2 == 0) {
      iVar2 = param_1 + 0x1c;
      sVar3 = _strlen [GHIDRA-AUTO-LABEL: unreliable]("orientation");
      FUN_00484fd0(param_2 + sVar3,iVar2);
    }
    pcVar1 = (char *)FUN_00484d10(param_2);
    iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("position",pcVar1);
    if (iVar2 == 0) {
      uVar7 = 0xffffffff;
      iVar2 = local_c + 8;
      iVar4 = param_1 + 0x44;
      iVar5 = param_1 + 0x38;
      sVar3 = _strlen [GHIDRA-AUTO-LABEL: unreliable]("position");
      uVar7 = FUN_00484d10(param_2,param_2 + sVar3,iVar5,iVar4,iVar2,uVar7);
      FUN_004861d0(uVar7);
    }
    else {
      pcVar1 = (char *)FUN_00484d10(param_2);
      iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("positionkey",pcVar1);
      if (iVar2 == 0) {
        iVar5 = local_8 + 0x10;
        iVar2 = local_c + 8;
        iVar4 = param_1 + 0x44;
        iVar6 = param_1 + 0x38;
        sVar3 = _strlen [GHIDRA-AUTO-LABEL: unreliable]("positionkey");
        uVar7 = FUN_00484d10(param_2,param_2 + sVar3,iVar6,iVar4,iVar2,iVar5);
        FUN_00486700(uVar7);
      }
      else {
        pcVar1 = (char *)FUN_00484d10(param_2);
        iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("positionbezierkey",pcVar1);
        if (iVar2 == 0) {
          iVar5 = local_8 + 0x10;
          iVar2 = local_c + 8;
          iVar4 = param_1 + 0x44;
          iVar6 = param_1 + 0x38;
          sVar3 = _strlen [GHIDRA-AUTO-LABEL: unreliable]("positionbezierkey");
          uVar7 = FUN_00484d10(param_2,param_2 + sVar3,iVar6,iVar4,iVar2,iVar5);
          FUN_00486900(uVar7);
        }
      }
    }
    pcVar1 = (char *)FUN_00484d10(param_2);
    iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("orientation",pcVar1);
    if (iVar2 == 0) {
/* ... 105 more lines truncated by --max-lines */
```

### decoy: 0x00486fb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00486fb0: 16-byte aligned: YES
  00486fb0  55             PUSH EBP
  00486fb1  8B EC          MOV EBP,ESP
  00486fb3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00486fb0
string literals (56):
  0x0098C8A0  at 00486fdc  'color'
  0x0098C8A0  at 00487004  'color'
  0x0098C894  at 0048703c  'colorkey'
  0x0098C894  at 00487064  'colorkey'
  0x0098C884  at 00487099  'colorbezierkey'
  0x0098C884  at 004870c1  'colorbezierkey'
  0x0098C87C  at 004870f4  'radius'
  0x0098C87C  at 0048711c  'radius'
  0x0098C870  at 00487154  'radiuskey'
  0x0098C870  at 0048717c  'radiuskey'
  0x0098C860  at 004871b1  'radiusbezierkey'
  0x0098C860  at 004871d9  'radiusbezierkey'
direct callees (12):
  0x00484D10  Global::AurMessageGetToken
  0x009370C7  _stricmp
  0x00919F10  _strlen
  0x004861D0  (unnamed)
  0x00486700  (unnamed)
  0x00486900  (unnamed)
  0x00484E20  (unnamed)
  0x00485A40  (unnamed)
  0x0048E8E0  (unnamed)
  0x00429010  (unnamed)
  0x00484E00  (unnamed)
  0x00486AA0  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 1295 B, RET 4
- callers 6: 0x00453b50, 0x00486fb0, 0x004878b0, 0x0048b6d0, 0x0048b6f0, ...
- callees: 0x00484d10 (CALL), 0x009370c7 _stricmp (CALL), 0x00919f10 _strlen (CALL), 0x00484d90 (CALL), 0x00484fd0 (CALL), 0x004861d0 (CALL), ...
- strings: 0x0098c660 "scalebezierkey", 0x0098c670 "scalekey", 0x0098c67c "scale", ...
- globals: 0x00a309bd READ
- twins: none

## Card B17-3-03

GOG function: CSWGuiOptionsGraphicsAdvanced::OnTexQualLeft  gog 0x0056d040  callconv=(blank)  param_size_bytes=4  gog_size=84  gog_retn=4
Candidate Steam address: 0x007d7730 (size 84, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.67) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007d7790:STR0/CALLEE0/SIG0.875/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0056cff0 CSWGuiOptionsGraphicsAdvanced::OnDefault  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0056d0a0 CSWGuiOptionsGraphicsAdvanced::OnTexQualRight  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0056d100 CSWGuiOptionsGraphicsAdvanced::SetTexQualText -> Steam ?  in candidate callees: n/a
  0x00910720 CSWGuiManager::PlayGuiSound -> Steam 0x004122a0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x1c*,0x50*,0x2920*} | Steam {0x1c*,0x50*,0x2920*}
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
prologue for 0x007d7730: 16-byte aligned: YES
  007d7730  55             PUSH EBP
  007d7731  8B EC          MOV EBP,ESP
  007d7733  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007d7730
string literals (0):
direct callees (2):
  0x007D77F0  (unnamed)
  0x004122A0  CSWGuiManager::PlayGuiSound
struct offsets touched (top 3):
  +0x2920 [dec 10528]  x3  via EAX,EDX
  +0x1C [dec 28]  x1  via EAX
  +0x50 [dec 80]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007d7730 FUN_007d7730 size=84 ret=4 callers=0 callees=2

void __thiscall FUN_007d7730(int param_1,int param_2)

{
  if ((*(int *)(param_2 + 0x50) != 0) && (0 < *(int *)(param_1 + 0x2920))) {
    *(int *)(param_1 + 0x2920) = *(int *)(param_1 + 0x2920) + -1;
    FUN_007d77f0();
    FUN_004122a0(1);
  }
  return;
}
```

### decoy: 0x007d7790 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007d7790: 16-byte aligned: YES
  007d7790  55             PUSH EBP
  007d7791  8B EC          MOV EBP,ESP
  007d7793  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007d7790
string literals (0):
direct callees (2):
  0x007D77F0  (unnamed)
  0x004122A0  CSWGuiManager::PlayGuiSound
struct offsets touched (top 4):
  +0x2920 [dec 10528]  x3  via ECX,EDX
  +0x1C [dec 28]  x1  via EDX
  +0x50 [dec 80]  x1  via EAX
  +0x2924 [dec 10532]  x1  via EAX
```

### Static facts (script output, report-only)
- size 84 B, RET 4
- callees: 0x007d77f0 (CALL), 0x004122a0 CSWGuiManager::PlayGuiSound (CALL)
- twins: none

## Card B17-3-04

GOG function: CAurInternal::ResetGammaRamp  gog 0x0086a750  callconv=(blank)  param_size_bytes=8  gog_size=356  gog_retn=8
Candidate Steam address: 0x00478300 (size 1251, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.40) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.06 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00478270:STR0/CALLEE0/SIG0.062/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0086a6d0 CAurInternalGL::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0086a8c0 Global::NewCAurora  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00921312 Global::@__security_check_cookie@4 -> Steam ?  in candidate callees: n/a
  0x00927220 Global::pow -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0xb0} | Steam {0x24,0xa8,0xac,0xb4}
imm sets (* = in both, 1 common): GOG {0x3,0x8*,0x9,0x10,0x100,0x628,0xc00} | Steam {0x4,0x8*,0x90}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00478300: 16-byte aligned: YES
  00478300  55             PUSH EBP
  00478301  8B EC          MOV EBP,ESP
  00478303  81 EC 90 00 00 00 SUB ESP,0x90
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00478300
string literals (0):
direct callees (5):
  0x005064F0  (unnamed)
  0x005065D0  (unnamed)
  0x009196FD  operator delete
  0x008339D0  (unnamed)
  0x00435720  (unnamed)
struct offsets touched (top 4):
  +0xB4 [dec 180]  x3  via EAX,ECX
  +0xA8 [dec 168]  x2  via ECX,EDX
  +0x24 [dec 36]  x1  via EDX
  +0xAC [dec 172]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00478300 FUN_00478300 size=1251 ret=8 callers=1 callees=9

void __thiscall FUN_00478300(int *param_1,int param_2,int param_3)

{
  int iVar1;
  
  FUN_005064f0();
  iVar1 = DAT_00a7bae0;
  if (DAT_00a7bae0 != 0) {
    if (DAT_00a7bae0 != 0) {
      FUN_005065d0();
      FUN_009196fd(iVar1);
    }
    DAT_00a7bae0 = 0;
  }
  iVar1 = DAT_00a7bae4;
  if (DAT_00a7bae4 != 0) {
    if (DAT_00a7bae4 != 0) {
      FUN_005065d0();
      FUN_009196fd(iVar1);
    }
    DAT_00a7bae4 = 0;
  }
  iVar1 = DAT_00a7bae8;
  if (DAT_00a7bae8 != 0) {
    if (DAT_00a7bae8 != 0) {
      FUN_005065d0();
      FUN_009196fd(iVar1);
    }
    DAT_00a7bae8 = 0;
  }
  iVar1 = DAT_00a736d0;
  if (DAT_00a736d0 != 0) {
    if (DAT_00a736d0 != 0) {
      FUN_005065d0();
      FUN_009196fd(iVar1);
    }
    DAT_00a736d0 = 0;
  }
  iVar1 = DAT_00a736d4;
  if (DAT_00a736d4 != 0) {
    if (DAT_00a736d4 != 0) {
      FUN_005065d0();
      FUN_009196fd(iVar1);
    }
    DAT_00a736d4 = 0;
  }
  iVar1 = DAT_00a736d8;
  if (DAT_00a736d8 != 0) {
    if (DAT_00a736d8 != 0) {
      FUN_005065d0();
      FUN_009196fd(iVar1);
    }
    DAT_00a736d8 = 0;
  }
  iVar1 = DAT_00a736dc;
  if (DAT_00a736dc != 0) {
    if (DAT_00a736dc != 0) {
      FUN_005065d0();
      FUN_009196fd(iVar1);
    }
    DAT_00a736dc = 0;
  }
  iVar1 = DAT_00a736e0;
  if (DAT_00a736e0 != 0) {
    if (DAT_00a736e0 != 0) {
      FUN_005065d0();
      FUN_009196fd(iVar1);
    }
    DAT_00a736e0 = 0;
  }
  iVar1 = DAT_00a7baec;
  if (DAT_00a7baec != 0) {
    if (DAT_00a7baec != 0) {
      FUN_005065d0();
      FUN_009196fd(iVar1);
    }
    DAT_00a7baec = 0;
  }
  iVar1 = DAT_00a7baf0;
/* ... 48 more lines truncated by --max-lines */
```

### decoy: 0x00478270 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00478270: 16-byte aligned: YES
  00478270  55             PUSH EBP
  00478271  8B EC          MOV EBP,ESP
  00478273  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00478270
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0xA0 [dec 160]  x1  via ECX
  +0xA4 [dec 164]  x1  via EAX
  +0xB4 [dec 180]  x1  via EAX
```

### Static facts (script output, report-only)
- size 1251 B, RET 8
- callers 1: 0x004787f0
- callees: 0x005064f0 (CALL), 0x005065d0 (CALL), 0x009196fd operator delete (CALL), 0x008339d0 (CALL), 0x00435720 (CALL), 0x00000045 (CALL), ...
- indirect: 0x004787b5 slot 0x24
- globals: 0x00a1b484 READ, 0x00a1b484 READ, 0x00a1b484 WRITE
- imports: ChangeDisplaySettingsA (USER32.DLL), DestroyWindow (USER32.DLL), wglDeleteContext (OPENGL32.DLL), ...
- twins: none

## Card B17-3-05

GOG function: CAurBehaviorCameraFollow::Control  gog 0x008c4b30  callconv=(blank)  param_size_bytes=4  gog_size=1703  gog_retn=4
Candidate Steam address: 0x00505970 (size 1067, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.08 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00505da0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008c4700 CAurBehaviorCameraFollow::Message  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008c51e0 CAurBehaviorCameraFollow::GetTargetWorldPosition  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x00413280 Vector::operator+ -> Steam ?  in candidate callees: n/a
  0x00419020 Vector::operator* -> Steam ?  in candidate callees: n/a
  0x0042a8f0 Quaternion::GetZAxis -> Steam ?  in candidate callees: n/a
  0x0080d120 Global::normalize_2 -> Steam ?  in candidate callees: n/a
  0x0080d220 Global::slerp -> Steam ?  in candidate callees: n/a
  0x0080d4b0 Global::YawPitchRoll -> Steam ?  in candidate callees: n/a
  0x008c31a0 CAurBehavior::Control -> Steam ?  in candidate callees: n/a
  0x00927220 Global::pow -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x8,0xc,0x10,0x14,0x30,0x34,0x38,0x3c,0x40,0x44,0x48,0x4c,0x50,...(+8)} | Steam {}
imm sets (* = in both, 4 common): GOG {0x4*,0x5,0x8*,0xc,0x10,0x18,0x24*,0x41,0x44*,0x198} | Steam {0x4*,0x8*,0x24*,0x30,0x34,0x38,0x3c,0x40,0x44*,0x48,0x4c,0x50,0x54,0x58,...(+5)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00505970: 16-byte aligned: YES
  00505970  55             PUSH EBP
  00505971  8B EC          MOV EBP,ESP
  00505973  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00505970
string literals (26):
  0x00991A94  at 005059c2  'spring'
  0x00991A94  at 005059da  'spring'
  0x00991A84  at 00505a3e  'offset'
  0x00991A84  at 00505a56  'offset'
  0x00991A78  at 00505aba  'pitch'
  0x00991A78  at 00505ad2  'pitch'
  0x0098D7A4  at 00505b36  'speed'
  0x0098D7A4  at 00505b4e  'speed'
  0x00991A70  at 00505b74  'minyaw'
  0x00991A70  at 00505b8c  'minyaw'
  0x00991A68  at 00505bb2  'maxyaw'
  0x00991A68  at 00505bca  'maxyaw'
direct callees (5):
  0x00484D10  Global::AurMessageGetToken
  0x009370C7  _stricmp
  0x00919F10  _strlen
  0x00484E00  (unnamed)
  0x00484D90  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00505970 FUN_00505970 size=1067 ret=4 callers=0 callees=5

void __thiscall FUN_00505970(int param_1,int param_2)

{
  char *pcVar1;
  int iVar2;
  size_t sVar3;
  
  pcVar1 = (char *)FUN_00484d10(param_2,param_1);
  iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("turn",pcVar1);
  if (iVar2 == 0) {
    iVar2 = param_1 + 0x40;
    sVar3 = _strlen [GHIDRA-AUTO-LABEL: unreliable]("turn");
    FUN_00484e00(sVar3 + param_2,iVar2);
  }
  pcVar1 = (char *)FUN_00484d10(param_2);
  iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("spring",pcVar1);
  if (iVar2 == 0) {
    iVar2 = param_1 + 0x38;
    sVar3 = _strlen [GHIDRA-AUTO-LABEL: unreliable]("spring");
    FUN_00484e00(sVar3 + param_2,iVar2);
  }
  pcVar1 = (char *)FUN_00484d10(param_2);
  iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("dist",pcVar1);
  if (iVar2 == 0) {
    iVar2 = param_1 + 0x30;
    sVar3 = _strlen [GHIDRA-AUTO-LABEL: unreliable]("dist");
    FUN_00484e00(sVar3 + param_2,iVar2);
  }
  pcVar1 = (char *)FUN_00484d10(param_2);
  iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("offset",pcVar1);
  if (iVar2 == 0) {
    iVar2 = param_1 + 0x24;
    sVar3 = _strlen [GHIDRA-AUTO-LABEL: unreliable]("offset");
    FUN_00484d90(sVar3 + param_2,iVar2);
  }
  pcVar1 = (char *)FUN_00484d10(param_2);
  iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("yaw",pcVar1);
  if (iVar2 == 0) {
    iVar2 = param_1 + 0x3c;
    sVar3 = _strlen [GHIDRA-AUTO-LABEL: unreliable]("yaw");
    FUN_00484e00(sVar3 + param_2,iVar2);
  }
  pcVar1 = (char *)FUN_00484d10(param_2);
  iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("pitch",pcVar1);
  if (iVar2 == 0) {
    iVar2 = param_1 + 0x44;
    sVar3 = _strlen [GHIDRA-AUTO-LABEL: unreliable]("pitch");
    FUN_00484e00(sVar3 + param_2,iVar2);
  }
  pcVar1 = (char *)FUN_00484d10(param_2);
  iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("tilt",pcVar1);
  if (iVar2 == 0) {
    iVar2 = param_1 + 0x48;
    sVar3 = _strlen [GHIDRA-AUTO-LABEL: unreliable]("tilt");
    FUN_00484e00(sVar3 + param_2,iVar2);
  }
  pcVar1 = (char *)FUN_00484d10(param_2);
  iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("speed",pcVar1);
  if (iVar2 == 0) {
    iVar2 = param_1 + 0x34;
    sVar3 = _strlen [GHIDRA-AUTO-LABEL: unreliable]("speed");
    FUN_00484e00(sVar3 + param_2,iVar2);
  }
  pcVar1 = (char *)FUN_00484d10(param_2);
  iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("minyaw",pcVar1);
  if (iVar2 == 0) {
    iVar2 = param_1 + 0x50;
    sVar3 = _strlen [GHIDRA-AUTO-LABEL: unreliable]("minyaw");
    FUN_00484e00(sVar3 + param_2,iVar2);
  }
  pcVar1 = (char *)FUN_00484d10(param_2);
  iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("maxyaw",pcVar1);
  if (iVar2 == 0) {
    iVar2 = param_1 + 0x54;
    sVar3 = _strlen [GHIDRA-AUTO-LABEL: unreliable]("maxyaw");
    FUN_00484e00(sVar3 + param_2,iVar2);
  }
  pcVar1 = (char *)FUN_00484d10(param_2);
  iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("minpitch",pcVar1);
/* ... 49 more lines truncated by --max-lines */
```

### decoy: 0x00505da0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00505da0: 16-byte aligned: YES
  00505da0  55             PUSH EBP
  00505da1  8B EC          MOV EBP,ESP
  00505da3  81 EC 98 01 00 00 SUB ESP,0x198
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00505da0
string literals (0):
direct callees (8):
  0x005043D0  (unnamed)
  0x0091E390  (unnamed)
  0x0044C8A0  Vector::operator+ (Vector3 add)
  0x00436200  (unnamed)
  0x00515620  (unnamed)
  0x00515390  (unnamed)
  0x00515290  (unnamed)
  0x00463120  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x12  via EAX,ECX,EDX
  +0x8 [dec 8]  x12  via EAX,ECX,EDX
  +0x10 [dec 16]  x10  via EAX,ECX,EDX
  +0x30 [dec 48]  x8  via EAX,ECX,EDX
  +0x3C [dec 60]  x8  via EAX,ECX,EDX
  +0x44 [dec 68]  x8  via EAX,ECX,EDX
  +0xC [dec 12]  x7  via EAX,ECX,EDX
  +0x60 [dec 96]  x5  via ECX,EDX
  +0x54 [dec 84]  x4  via EAX,ECX
  +0x68 [dec 104]  x4  via EAX,ECX,EDX
```

### Static facts (script output, report-only)
- size 1067 B, RET 4
- callees: 0x00484d10 (CALL), 0x009370c7 _stricmp (CALL), 0x00919f10 _strlen (CALL), 0x00484e00 (CALL), 0x00484d90 (CALL)
- strings: 0x00991a1c "clampdist", 0x00991a28 "clamppitch", 0x00991a34 "clampyaw", ...
- twins: none

## Card B17-3-06

GOG function: CSWGui3DSceneView::Draw  gog 0x0053c050  callconv=(blank)  param_size_bytes=4  gog_size=37  gog_retn=4
Candidate Steam address: 0x0084de40 (size 37, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0084de70:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0053bff0 CSWGuiMainMenu::OnMoreGamesPressed  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0053c080 CSWGui3DSceneView::`scalar  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0xc*,0x60*} | Steam {0xc*,0x60*}
imm sets (* = in both, 2 common): GOG {0x4*,0x60*} | Steam {0x4*,0x60*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0084de40: 16-byte aligned: YES
  0084de40  55             PUSH EBP
  0084de41  8B EC          MOV EBP,ESP
  0084de43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0084de40
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0xC [dec 12]  x1  via EDX
  +0x60 [dec 96]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0084de40 FUN_0084de40 size=37 ret=4 callers=0 callees=0

void __thiscall FUN_0084de40(int param_1,undefined4 param_2)

{
  (**(code **)(*(int *)(param_1 + 0x60) + 0xc))(param_2);
  return;
}
```

### decoy: 0x0084de70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0084de70: 16-byte aligned: YES
  0084de70  55             PUSH EBP
  0084de71  8B EC          MOV EBP,ESP
  0084de73  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0084de70
string literals (0):
direct callees (3):
  0x00418130  (unnamed)
  0x00418640  CSWGuiControl::Destructor
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 37 B, RET 4
- indirect: 0x0084de5d slot 0xc
- twins: none

## Card B17-3-07

GOG function: CSWPlayerControlCamRelative::GetPlayerWalking  gog 0x004f0520  callconv=__thiscall  param_size_bytes=0  gog_size=17  gog_retn=0
Candidate Steam address: 0x00726f80 (size 17, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 17 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 17 (callers: GetModuleCamera 0x781810, MainLoop 0x781be0, UpdateRoom 0x783dc0)
- best decoy (neighbour/other candidate) scores: 0x00726f60:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004f0500 CSWPlayerControlCamRelative::SetPlayerWalking  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004f0540 CSWPlayerControlCamRelative::GetCurSpeed  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (18 of 20): the card's A-CALLEE credit may come from these, not from callees
  0x0040c8f0 CClientExoApp::GetModule -> Steam 0x0073f430  in candidate callers: yes
  0x0042dd90 CClientExoAppInternal::GetModuleCamera -> Steam 0x00781810  in candidate callers: yes
  0x0042e160 CClientExoAppInternal::MainLoop -> Steam 0x00781be0  in candidate callers: yes
  0x00430340 CClientExoAppInternal::UpdateRoom -> Steam 0x00783dc0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x18*} | Steam {0x18*}
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
prologue for 0x00726f80: 16-byte aligned: YES
  00726f80  55             PUSH EBP
  00726f81  8B EC          MOV EBP,ESP
  00726f83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00726f80
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x18 [dec 24]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00726f80 FUN_00726f80 size=17 ret=0 callers=20 callees=0

undefined4 __fastcall FUN_00726f80(int param_1)

{
  return *(undefined4 *)(param_1 + 0x18);
}
```

### decoy: 0x00726f60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00726f60: 16-byte aligned: YES
  00726f60  55             PUSH EBP
  00726f61  8B EC          MOV EBP,ESP
  00726f63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00726f60
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x14 [dec 20]  x1  via EAX
```

### Static facts (script output, report-only)
- size 17 B, RET 0
- callers 20: 0x00721bb0 CERFFile::WriteResource_2, 0x00722a30, 0x0073f430 CClientExoApp::GetModule, 0x00781810 CClientExoAppInternal::GetModuleCamera, 0x00781be0 CClientExoAppInternal::MainLoop, ...
- twins: none

## Card B17-3-08

GOG function: InitControl::ScaleToEdgeHelper_2  gog 0x0090deb0  callconv=(blank)  param_size_bytes=12  gog_size=79  gog_retn=0
Candidate Steam address: 0x0040fb70 (size 209, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0040fc50:STR0/CALLEE0/SIG0.062/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0090de70 InitControl::ScaleToEdgeHelper  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0090df00 InitControl::ScaleToEdgeHelper_3 -> 0x0040fa80  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x24,0x28,0x4c}
imm sets (* = in both, 0 common): GOG {0x2,0x5} | Steam {0x4,0x1c}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 0)
```

### prologue
```
prologue for 0x0040fb70: 16-byte aligned: YES
  0040fb70  55             PUSH EBP
  0040fb71  8B EC          MOV EBP,ESP
  0040fb73  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0040fb70
string literals (0):
direct callees (2):
  0x00418720  (unnamed)
  0x004186F0  (unnamed)
struct offsets touched (top 3):
  +0x24 [dec 36]  x3  via EAX,ECX
  +0x28 [dec 40]  x1  via EAX
  +0x4C [dec 76]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0040fb70 FUN_0040fb70 size=209 ret=0 callers=1 callees=2

void __fastcall FUN_0040fb70(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 local_18;
  undefined4 local_c;
  
  iVar1 = *(int *)(param_1 + 0x28);
  for (local_c = 0; local_c < iVar1; local_c = local_c + 1) {
    if (*(int *)(*(int *)(param_1 + 0x24) + local_c * 4) != 0) {
      iVar2 = (**(code **)(**(int **)(*(int *)(param_1 + 0x24) + local_c * 4) + 0x4c))();
      if (iVar2 != 0) {
        for (local_18 = 0; local_18 < 4; local_18 = local_18 + 1) {
          iVar2 = FUN_00418720(local_18);
          if ((iVar2 < 0) || (iVar1 <= iVar2)) {
            FUN_004186f0(local_18,0);
          }
          else {
            FUN_004186f0(local_18,*(undefined4 *)(*(int *)(param_1 + 0x24) + iVar2 * 4));
          }
        }
      }
    }
  }
  return;
}
```

### decoy: 0x0040fc50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0040fc50: 16-byte aligned: YES
  0040fc50  55             PUSH EBP
  0040fc51  8B EC          MOV EBP,ESP
  0040fc53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0040fc50
string literals (5):
  0x009874D8  at 0040fc7f  '800x600'
  0x009874CC  at 0040fc9f  '1600x1200'
  0x009874C0  at 0040fcba  '1280x1024'
  0x009874B4  at 0040fcd5  '1280x960'
  0x009874A8  at 0040fcf0  '1024x768'
direct callees (6):
  0x00733570  CExoString::CExoString_char
  0x007338D0  CExoString::operator=_2
  0x00733DF0  CExoString::operator+
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x007335F0  CExoString::CExoString_copy
struct offsets touched (top 1):
  +0x4C [dec 76]  x1  via EAX
```

### Static facts (script output, report-only)
- size 209 B, RET 0
- callers 1: 0x0040f5a0 CSWGuiPanel::StopLoadFromLayout
- callees: 0x00418720 (CALL), 0x004186f0 (CALL)
- indirect: 0x0040fbcd slot 0x4c
- twins: none

## Card B17-3-09

GOG function: InitControl::ScaleToEdgeHelper_2  gog 0x0090deb0  callconv=(blank)  param_size_bytes=12  gog_size=79  gog_retn=0
Candidate Steam address: 0x0040f5a0 (size 117, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CSWGuiPanel::StopLoadFromLayout | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0040f620:STR0/CALLEE0/SIG0.042/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0090de70 InitControl::ScaleToEdgeHelper  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0090df00 InitControl::ScaleToEdgeHelper_3 -> 0x0040fa80  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x30,0x48}
imm sets (* = in both, 0 common): GOG {0x2,0x5} | Steam {0x10,0xfffffffd}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 0)
```

### prologue
```
prologue for 0x0040f5a0: 16-byte aligned: YES
  0040f5a0  55             PUSH EBP
  0040f5a1  8B EC          MOV EBP,ESP
  0040f5a3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0040f5a0
string literals (0):
direct callees (2):
  0x007112B0  CRes::Release
  0x0040FB70  (unnamed)
struct offsets touched (top 2):
  +0x30 [dec 48]  x3  via EAX,ECX,EDX
  +0x48 [dec 72]  x3  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0040f5a0 FUN_0040f5a0 size=117 ret=0 callers=73 callees=2

void __fastcall FUN_0040f5a0(int param_1)

{
  undefined4 uVar1;
  
  if ((*(uint *)(param_1 + 0x48) >> 1 & 1) != 0) {
    FUN_007112b0();
    if (*(undefined4 **)(param_1 + 0x30) == (undefined4 *)0x0) {
      uVar1 = 0;
    }
    else {
      uVar1 = (**(code **)**(undefined4 **)(param_1 + 0x30))(1);
    }
    *(undefined4 *)(param_1 + 0x30) = 0;
    *(uint *)(param_1 + 0x48) = *(uint *)(param_1 + 0x48) & 0xfffffffd;
    FUN_0040fb70(uVar1);
  }
  return;
}
```

### decoy: 0x0040f620 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0040f620: 16-byte aligned: YES
  0040f620  55             PUSH EBP
  0040f621  8B EC          MOV EBP,ESP
  0040f623  81 EC AC 00 00 00 SUB ESP,0xac
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0040f620
string literals (1):
  0x00987480  at 0040f90d  'LBL_BAR'
direct callees (9):
  0x00418DF0  CSWGuiControl::Load_2
  0x0073FEA0  CClientExoApp::GetSWGuiManager
  0x0091C860  _ftol2_sse
  0x0040F9F0  (unnamed)
  0x0040FA80  (unnamed)
  0x0040FB40  (unnamed)
  0x00734010  CExoString::CStr
  0x0091D6F0  (unnamed)
  0x0083EA60  CExoArrayList<uint32>::Add
struct offsets touched (top 10):
  +0x4 [dec 4]  x8  via EAX,ECX,EDX
  +0x20 [dec 32]  x2  via EAX,EDX
  +0x28 [dec 40]  x2  via EAX,EDX
  +0x6C [dec 108]  x2  via EAX
  +0x6E [dec 110]  x2  via ECX,EDX
  +0x8 [dec 8]  x1  via EDX
  +0xC [dec 12]  x1  via EDX
  +0x24 [dec 36]  x1  via EDX
  +0x30 [dec 48]  x1  via EDX
  +0x50 [dec 80]  x1  via EAX
```

### Static facts (script output, report-only)
- size 117 B, RET 0
- callers 73: 0x0040e860 CSWGuiPanel::Destructor, 0x00747210, 0x00754ed0, 0x00757c40, 0x0075ae40, ...
- callees: 0x007112b0 CRes::Release (CALL), 0x0040fb70 (CALL)
- indirect: 0x0040f5e2 slot 0x0
- twins: none

## Card B17-3-10

GOG function: CSWGuiOptionsSoundAdvanced::SetNewOptions  gog 0x00571320  callconv=(blank)  param_size_bytes=0  gog_size=129  gog_retn=0
Candidate Steam address: 0x007dba10 (size 129, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007db680:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00570f90 CSWGuiOptionsSoundAdvanced::HandleInputEvent  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005713b0 CSWGuiOptionsSoundAdvanced::OnEAXLeft  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0040ccf0 CClientExoApp::GetClientOptions -> Steam 0x0072fb00  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x4*,0x51*,0x54*,0x330*,0x18b4*} | Steam {0x4*,0x51*,0x54*,0x330*,0x18b4*}
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
prologue for 0x007dba10: 16-byte aligned: YES
  007dba10  55             PUSH EBP
  007dba11  8B EC          MOV EBP,ESP
  007dba13  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007dba10
string literals (0):
direct callees (1):
  0x0072FB00  CClientExoApp::GetClientOptions
struct offsets touched (top 5):
  +0x4 [dec 4]  x3  via EAX,EDX
  +0x51 [dec 81]  x1  via EAX
  +0x54 [dec 84]  x1  via EAX
  +0x330 [dec 816]  x1  via ECX
  +0x18B4 [dec 6324]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007dba10 FUN_007dba10 size=129 ret=0 callers=1 callees=1

void __fastcall FUN_007dba10(int param_1)

{
  undefined1 uVar1;
  undefined4 uVar2;
  int iVar3;
  
  iVar3 = FUN_0072fb00();
  if (iVar3 != 0) {
    uVar1 = *(undefined1 *)(param_1 + 0x330);
    iVar3 = FUN_0072fb00();
    *(undefined1 *)(iVar3 + 0x51) = uVar1;
    uVar2 = *(undefined4 *)(param_1 + 0x18b4);
    iVar3 = FUN_0072fb00();
    *(undefined4 *)(iVar3 + 0x54) = uVar2;
  }
  return;
}
```

### decoy: 0x007db680 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007db680: 16-byte aligned: YES
  007db680  55             PUSH EBP
  007db681  8B EC          MOV EBP,ESP
  007db683  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007db680
string literals (5):
  0x00986BA8  at 007db7e8  'Sound Options'
  0x00986B98  at 007db7f9  '.\\\\swkotor2.ini'
  0x0099F888  at 007db8a8  'Force Software'
  0x00986BA8  at 007db8b9  'Sound Options'
  0x00986B98  at 007db8ca  '.\\\\swkotor2.ini'
direct callees (15):
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x007DBDE0  (unnamed)
  0x007DBA10  (unnamed)
  0x0072FB00  CClientExoApp::GetClientOptions
  0x0070B9B0  (unnamed)
  0x00736240  (unnamed)
  0x00733540  CExoString::Constructor
  0x0070BB80  (unnamed)
  0x00734270  CExoString::Format
  0x00733570  CExoString::CExoString_char
  0x00736350  (unnamed)
  0x00733780  CExoString::~CExoString
  0x007362C0  (unnamed)
  0x004109C0  CSWGuiManager::PopModalPanel
  0x0040EAA0  CSWGuiPanel::HandleInputEvent
struct offsets touched (top 10):
  +0x1C [dec 28]  x4  via EAX,ECX,EDX
  +0x48 [dec 72]  x4  via ECX,EDX
  +0x4 [dec 4]  x3  via EAX,ECX
  +0x3C [dec 60]  x2  via EAX,EDX
  +0x54 [dec 84]  x2  via EAX,EDX
  +0xB34 [dec 2868]  x2  via EAX,EDX
  +0x18B8 [dec 6328]  x2  via EDX
  +0x18BC [dec 6332]  x2  via EAX,ECX
  +0x51 [dec 81]  x1  via EDX
  +0x330 [dec 816]  x1  via EDX
```

### Static facts (script output, report-only)
- size 129 B, RET 0
- callers 1: 0x007db680
- callees: 0x0072fb00 CClientExoApp::GetClientOptions (CALL)
- twins: none

## Card B17-3-11

GOG function: CSWSpell::GetAdjustedForcePoints  gog 0x004f0120  callconv=(blank)  param_size_bytes=17  gog_size=408  gog_retn=20
Candidate Steam address: 0x006c4120 (size 408, retn 20)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.82) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 4 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 4 (callers: ? 0x5482a0, GetSpellUsesLeft 0x6b8f40, GetCreatureTalentRandomKnownPower 0x6bcfa0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006c3fe0:STR0/CALLEE0/SIG0.115/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004effe0 CSWSpell::SetMasterSpells  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004f02c0 CSWPlayerControl::GetUpDown  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00620e90 Global::GetFLOATEntry -> Steam ?  in candidate callees: n/a
  0x00924490 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (4 of 8): the card's A-CALLEE credit may come from these, not from callees
  0x006aef80 CSWSObject::SpellCastAndImpact -> Steam 0x005482a0  in candidate callers: yes
  0x006f53c0 CSWSCreatureStats::GetSpellUsesLeft -> Steam 0x006b8f40  in candidate callers: yes
  0x006f9420 CSWSCreatureStats::GetCreatureTalentRandomKnownPower -> Steam 0x006bcfa0  in candidate callers: yes
  0x006f9e40 CSWSCreatureStats::HasSpell -> Steam 0x006bd9c0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0xd4*,0x130*,0x15a*,0x15b*} | Steam {0xd4*,0x130*,0x15a*,0x15b*}
imm sets (* = in both, 9 common): GOG {0x5*,0xa*,0x14*,0x28*,0x41*,0x45*,0x47*,0x10a*,0x10c*} | Steam {0x5*,0xa*,0x14*,0x28*,0x41*,0x45*,0x47*,0x10a*,0x10c*}

### abi
```
GOG param bytes: 17; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(17/4)*4 = 20 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: TRUE (GOG 17 bytes -> x86 slots 20; ASM RET 20)
```

### prologue
```
prologue for 0x006c4120: 16-byte aligned: YES
  006c4120  55             PUSH EBP
  006c4121  8B EC          MOV EBP,ESP
  006c4123  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006c4120
string literals (0):
direct callees (2):
  0x0071B3F0  (unnamed)
  0x0091C860  _ftol2_sse
struct offsets touched (top 4):
  +0xD4 [dec 212]  x2  via ECX,EDX
  +0x130 [dec 304]  x2  via EAX,EDX
  +0x15B [dec 347]  x2  via EAX,EDX
  +0x15A [dec 346]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 006c4120 FUN_006c4120 size=408 ret=20 callers=8 callees=2

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint __thiscall
FUN_006c4120(int param_1,uint param_2,int param_3,int param_4,int param_5,char param_6)

{
  float local_c;
  uint local_8;
  
  local_8 = (uint)*(byte *)(param_1 + 0x15a);
  if (local_8 == 0) {
    local_8 = 0;
  }
  else {
    param_2 = param_2 / 10;
    if (param_3 != 0) {
      local_c = 1.0;
      if (*(char *)(param_1 + 0x15b) == 'G') {
        FUN_0071b3f0(param_2,DAT_00a0fe54,&local_c);
      }
      else if (*(char *)(param_1 + 0x15b) == 'E') {
        FUN_0071b3f0(param_2,DAT_00a0fe58,&local_c);
      }
      if ((((float)_DAT_00986b50 < local_c) && ('\0' < param_6)) &&
         (local_c = local_c - (float)(int)param_6 * (float)_DAT_0098bd30,
         local_c < (float)_DAT_00986b50)) {
        local_c = 1.0;
      }
      local_8 = FUN_0091c860();
    }
    if (param_2 < 6) {
      if (param_2 < 5) {
        local_8 = local_8 + param_4 / 0x14;
      }
    }
    else {
      local_8 = local_8 - param_4 / 0x14;
    }
    if ((int)local_8 < 1) {
      local_8 = 1;
    }
    if ((param_5 == 0x10a) || (param_5 == 0x10c)) {
      local_8 = FUN_0091c860();
    }
  }
  return local_8;
}
```

### decoy: 0x006c3fe0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006c3fe0: 16-byte aligned: YES
  006c3fe0  55             PUSH EBP
  006c3fe1  8B EC          MOV EBP,ESP
  006c3fe3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006c3fe0
string literals (0):
direct callees (3):
  0x007337C0  CExoString::operator=
  0x00919F10  _strlen
  0x00733DB0  CExoString::operator[]
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 408 B, RET 20
- callers 8: 0x005482a0, 0x006b8f40 CSWSCreatureStats::GetSpellUsesLeft, 0x006b91b0 CSWSCreatureStats::UnReadySpell, 0x006bcfa0 CSWSCreatureStats::GetCreatureTalentRandomKnownPower, 0x006bd9c0 CSWSCreatureStats::HasSpell, ...
- callees: 0x0071b3f0 (CALL), 0x0091c860 (CALL)
- globals: 0x00a1b4d0 READ, 0x00a1b4d0 READ
- twins: none

## Card B17-3-12

GOG function: CAurGUIStringInternal::SetSizeBoundingRect_2  gog 0x008d10b0  callconv=(blank)  param_size_bytes=8  gog_size=89  gog_retn=8
Candidate Steam address: 0x0047a740 (size 83, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0047a7a0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008d1050 CAurGUIStringInternal::GetSizeBoundingRect_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008d1110 CAurGUIStringInternal::DrawBoundingRect  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00924490 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0xc*,0x10*} | Steam {0xc*,0x10*}
imm sets (* = in both, 3 common): GOG {0x8*,0xa*,0xc*} | Steam {0x8*,0xa*,0xc*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0047a740: 16-byte aligned: YES
  0047a740  55             PUSH EBP
  0047a741  8B EC          MOV EBP,ESP
  0047a743  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0047a740
string literals (0):
direct callees (0):
struct offsets touched (top 4):
  +0xC [dec 12]  x1  via EAX
  +0x10 [dec 16]  x1  via ECX
  +0xA309CC [dec 10684876]  x1  via ECX
  +0xA309CE [dec 10684878]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0047a740 FUN_0047a740 size=83 ret=8 callers=0 callees=0

void __thiscall FUN_0047a740(int param_1,float *param_2,float *param_3)

{
  *param_2 = (float)*(int *)(param_1 + 0xc) /
             (float)(int)*(short *)(&DAT_00a309cc + DAT_00a309c0 * 10);
  *param_3 = (float)*(int *)(param_1 + 0x10) / (float)(int)(short)(&DAT_00a309ce)[DAT_00a309c0 * 5];
  return;
}
```

### decoy: 0x0047a7a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0047a7a0: 16-byte aligned: YES
  0047a7a0  55             PUSH EBP
  0047a7a1  8B EC          MOV EBP,ESP
  0047a7a3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0047a7a0
string literals (0):
direct callees (1):
  0x0091C860  _ftol2_sse
struct offsets touched (top 4):
  +0xC [dec 12]  x1  via EDX
  +0x10 [dec 16]  x1  via EDX
  +0xA309CC [dec 10684876]  x1  via EAX
  +0xA309CE [dec 10684878]  x1  via EAX
```

### Static facts (script output, report-only)
- size 83 B, RET 8
- globals: 0x00a309c0 READ, 0x00a309c0 READ
- twins: none

## Card B17-3-13

GOG function: CSWCVisualEffectOnObject::ApplyBumpedOutShell  gog 0x005047f0  callconv=(blank)  param_size_bytes=2  gog_size=786  gog_retn=4
Candidate Steam address: 0x00878f10 (size 786, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.83) | G8 mac_rotation: pass
- anchors: A-STR 26 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,CALLEE,SIG  (content level C2)
- evidence summary: STR '?fx_tex_stealth'@009a5357; 'fx_tex_26'@009a5368; 'fx_tex_25'@009a5374; CALLEE 1 (callees: ? 0x7f2450); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00879290:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005042d0 CSWCVisualEffectOnObject::AttachLight  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00504b70 CSWCVisualEffectOnObject::RemoveBumpedOutShell  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x0040c9f0 CClientExoApp::GetItemByGameObjectID -> Steam 0x0073f530  in candidate callees: yes
  0x0041a480 CSWCCreature::GetEquippedItemID -> Steam 0x0076e610  in candidate callees: yes
  0x004775f0 CSWCObject::EnableBumpedOutShell -> Steam 0x007f2450  in candidate callees: yes
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x005036a0 CSWCVisualEffectOnObject::GetTargetCSWCObject -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x006059e0 CExoString::operator=_2 -> Steam ?  in candidate callees: n/a
  0x0061a160 CResRef::Constructor_3 -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (26): '?fx_tex_stealth'; 'fx_tex_26'; 'fx_tex_25'; 'fx_tex_24' ...
disp sets (* = in both, 3 common): GOG {0x4*,0x2c*,0xd8*} | Steam {0x4*,0x2c*,0xd8*}
imm sets (* = in both, 6 common): GOG {0x4*,0x10*,0x14*,0x19*,0x2c*,0x578*} | Steam {0x4*,0x10*,0x14*,0x19*,0x2c*,0x578*}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00878f10: 16-byte aligned: YES
  00878f10  55             PUSH EBP
  00878f11  8B EC          MOV EBP,ESP
  00878f13  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00878f10
string literals (25):
  0x009A5488  at 00878fb1  'fx_tex_01'
  0x009A547C  at 00878fc3  'fx_tex_02'
  0x009A5470  at 00878fd5  'fx_tex_03'
  0x009A5464  at 00878fe7  'fx_tex_04'
  0x009A5458  at 00878ff9  'fx_tex_05'
  0x009A544C  at 0087900b  'fx_tex_06'
  0x009A5440  at 0087901d  'fx_tex_07'
  0x009A5434  at 0087902f  'fx_tex_08'
  0x009A5428  at 00879041  'fx_tex_09'
  0x009A541C  at 00879053  'fx_tex_10'
  0x009A5410  at 00879065  'fx_tex_11'
  0x009A5404  at 00879077  'fx_tex_12'
direct callees (8):
  0x00733540  CExoString::Constructor
  0x00877DC0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x007338D0  CExoString::operator=_2
  0x00710810  (unnamed)
  0x007F2450  (unnamed)
  0x0076E610  CSWCCreature::GetEquippedItemID
  0x0073F530  CClientExoApp::GetItemByGameObjectID
struct offsets touched (top 3):
  +0x4 [dec 4]  x1  via ECX
  +0x2C [dec 44]  x1  via EDX
  +0xD8 [dec 216]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00878f10 FUN_00878f10 size=786 ret=4 callers=2 callees=8

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00878f10(short param_1)

{
  undefined1 auStack_54 [12];
  undefined4 uStack_48;
  char *local_44;
  uint uStack_40;
  int local_3c;
  int local_38;
  undefined4 local_34;
  undefined1 *local_30;
  undefined1 *local_2c;
  int local_28;
  int local_24;
  undefined1 local_20 [8];
  int *local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095abb8;
  local_10 = ExceptionList;
  uStack_40 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_44 = (char *)0x878f40;
  FUN_00733540();
  local_8 = 0;
  local_14 = _DAT_00991608;
  *(undefined1 *)(local_38 + 0xd8) = 0x14;
  local_44 = (char *)0x878f62;
  local_18 = (int *)FUN_00877dc0();
  if (local_18 == (int *)0x0) {
    local_8 = 0xffffffff;
    local_44 = (char *)0x878f7a;
    FUN_00733780();
  }
  else {
    local_3c = (ushort)(param_1 - 0x578) - 1;
    switch(local_3c) {
    case 0:
      local_44 = "fx_tex_01";
      uStack_48 = 0x878fbe;
      FUN_007338d0();
      break;
    case 1:
      local_44 = "fx_tex_02";
      uStack_48 = 0x878fd0;
      FUN_007338d0();
      break;
    case 2:
      local_44 = "fx_tex_03";
      uStack_48 = 0x878fe2;
      FUN_007338d0();
      break;
    case 3:
      local_44 = "fx_tex_04";
      uStack_48 = 0x878ff4;
      FUN_007338d0();
      break;
    case 4:
      local_44 = "fx_tex_05";
      uStack_48 = 0x879006;
      FUN_007338d0();
      break;
    case 5:
      local_44 = "fx_tex_06";
      uStack_48 = 0x879018;
      FUN_007338d0();
      break;
    case 6:
      local_44 = "fx_tex_07";
      uStack_48 = 0x87902a;
      FUN_007338d0();
      break;
    case 7:
/* ... 128 more lines truncated by --max-lines */
```

### decoy: 0x00879290 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00879290: 16-byte aligned: YES
  00879290  55             PUSH EBP
  00879291  8B EC          MOV EBP,ESP
  00879293  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00879290
string literals (0):
direct callees (4):
  0x00877DC0  (unnamed)
  0x007F2490  CSWCObject::DisableBumpedOutShell
  0x0076E610  CSWCCreature::GetEquippedItemID
  0x0073F530  CClientExoApp::GetItemByGameObjectID
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via ECX
  +0x2C [dec 44]  x1  via EDX
```

### Static facts (script output, report-only)
- size 786 B, RET 4
- callers 2: 0x00872910, 0x00878190
- callees: 0x00733540 CExoString::DefaultConstructor (CALL), 0x00877dc0 (CALL), 0x00733780 CExoString::Destructor (CALL), 0x007338d0 (CALL), 0x00710810 (CALL), 0x007f2450 (CALL), ...
- indirect: 0x008791ac slot 0x2c
- strings: 0x009a5368 "fx_tex_26", 0x009a5374 "fx_tex_25", 0x009a5380 "fx_tex_24", ...
- twins: none

## Card B17-3-14

GOG function: CSWGuiPazaakCard::HandleLMouseUp  gog 0x0054cfa0  callconv=(blank)  param_size_bytes=0  gog_size=120  gog_retn=0
Candidate Steam address: 0x008868d0 (size 120, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.60) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00886820:STR0/CALLEE0/SIG0.146/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0054cef0 CSWGuiPazaakCard::HandleMouseCapturedMovement  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0054d020 CSWGuiTutorial::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x009118e0 CSWGuiManager::ReleaseMouse -> Steam ?  in candidate callees: n/a
  0x00917430 CSWGuiControl::HandleLMouseUp -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x8*,0x14*,0x3c*,0x54*} | Steam {0x8*,0x14*,0x3c*,0x54*}
imm sets (* = in both, 2 common): GOG {0x10*,0x17feb*} | Steam {0x10*,0x17feb*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x008868d0: 16-byte aligned: YES
  008868d0  55             PUSH EBP
  008868d1  8B EC          MOV EBP,ESP
  008868d3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008868d0
string literals (0):
direct callees (2):
  0x00413460  (unnamed)
  0x00419190  (unnamed)
struct offsets touched (top 4):
  +0x54 [dec 84]  x2  via EAX,ECX
  +0x8 [dec 8]  x1  via EDX
  +0x14 [dec 20]  x1  via EAX
  +0x3C [dec 60]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 008868d0 FUN_008868d0 size=120 ret=0 callers=0 callees=2

void __fastcall FUN_008868d0(int *param_1)

{
  if (*(int **)(DAT_00a1b49c + 0x54) == param_1) {
    *(undefined4 *)(DAT_00a1b49c + 0x54) = 0;
    (**(code **)(*param_1 + 0x3c))(0x17feb,1);
    FUN_00413460(param_1);
    (**(code **)(*param_1 + 0x14))();
  }
  else {
    FUN_00419190();
  }
  return;
}
```

### decoy: 0x00886820 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00886820: 16-byte aligned: YES
  00886820  55             PUSH EBP
  00886821  8B EC          MOV EBP,ESP
  00886823  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00886820
string literals (0):
direct callees (2):
  0x00458940  identity (basic vtable slot 0x4)
  0x0091FBF2  (unnamed)
struct offsets touched (top 5):
  +0x54 [dec 84]  x2  via EAX
  +0xC [dec 12]  x1  via ECX
  +0x324 [dec 804]  x1  via ECX
  +0x328 [dec 808]  x1  via EAX
  +0x32C [dec 812]  x1  via EAX
```

### Static facts (script output, report-only)
- size 120 B, RET 0
- callees: 0x00413460 (CALL), 0x00419190 (CALL)
- indirect: 0x0088691c slot 0x3c, 0x00886938 slot 0x14
- globals: 0x00a1b49c READ, 0x00a1b49c READ, 0x00a1b49c READ
- twins: none

## Card B17-3-15

GOG function: CSWCAnimBaseTW::ReplaceTexture  gog 0x004f7db0  callconv=(blank)  param_size_bytes=12  gog_size=323  gog_retn=12
Candidate Steam address: 0x0086c460 (size 323, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.88) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0086c5b0:STR0/CALLEE0/SIG0.722/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004f7bb0 CSWCAnimBaseTW::LoadModel  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004f7f00 CSWCAnimBaseTW::ReplaceTexture_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x004c7ce0 CSWCAnimBase::ReplaceTexture -> Steam ?  in candidate callees: n/a
  0x0061a2d0 CResRef::CopyToString_2 -> Steam ?  in candidate callees: n/a
  0x0061a5f0 CResRef::operator= -> Steam ?  in candidate callees: n/a
  0x00921312 Global::@__security_check_cookie@4 -> Steam ?  in candidate callees: n/a
  0x00945a73 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x34*,0x138*,0x14c*,0x178*,0x17c*} | Steam {0x34*,0x138*,0x14c*,0x178*,0x17c*}
imm sets (* = in both, 5 common): GOG {0x4*,0xc*,0x50*,0x128*,0x13c*} | Steam {0x4*,0xc*,0x50*,0x128*,0x13c*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x0086c460: 16-byte aligned: YES
  0086c460  55             PUSH EBP
  0086c461  8B EC          MOV EBP,ESP
  0086c463  83 EC 50       SUB ESP,0x50
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0086c460
string literals (0):
direct callees (5):
  0x00710980  (unnamed)
  0x00946714  (unnamed)
  0x00710CA0  (unnamed)
  0x0085FE10  CSWCAnimBase::ReplaceTexture
  0x009196EE  __security_check_cookie
struct offsets touched (top 5):
  +0x178 [dec 376]  x3  via EAX,ECX
  +0x17C [dec 380]  x3  via ECX,EDX
  +0x34 [dec 52]  x2  via EDX
  +0x138 [dec 312]  x1  via EAX
  +0x14C [dec 332]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0086c460 FUN_0086c460 size=323 ret=12 callers=2 callees=5

void __thiscall FUN_0086c460(int param_1,undefined4 param_2,undefined4 param_3,int param_4)

{
  undefined1 local_50 [16];
  undefined1 local_40 [16];
  char local_30 [20];
  char local_1c [20];
  uint local_8;
  
  local_8 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  if (*(int *)(param_1 + 0x178) != 0) {
    FUN_00710980(local_30);
    FUN_00710980(local_1c);
    __strlwr [GHIDRA-AUTO-LABEL: unreliable](local_1c);
    __strlwr [GHIDRA-AUTO-LABEL: unreliable](local_30);
    (**(code **)(**(int **)(param_1 + 0x178) + 0x34))(local_30,local_1c);
    if (param_4 != 0) {
      FUN_00710ca0(local_40,param_3);
      *(undefined4 *)(param_1 + 0x138) = 0;
    }
  }
  if (*(int *)(param_1 + 0x17c) != 0) {
    FUN_00710980(local_30);
    FUN_00710980(local_1c);
    __strlwr [GHIDRA-AUTO-LABEL: unreliable](local_1c);
    __strlwr [GHIDRA-AUTO-LABEL: unreliable](local_30);
    (**(code **)(**(int **)(param_1 + 0x17c) + 0x34))(local_30,local_1c);
    if (param_4 != 0) {
      FUN_00710ca0(local_50,param_3);
      *(undefined4 *)(param_1 + 0x14c) = 0;
    }
  }
  FUN_0085fe10(param_2,param_3,param_4);
  __security_check_cookie [GHIDRA-AUTO-LABEL: unreliable](local_8 ^ (uint)&stack0xfffffffc);
  return;
}
```

### decoy: 0x0086c5b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0086c5b0: 16-byte aligned: YES
  0086c5b0  55             PUSH EBP
  0086c5b1  8B EC          MOV EBP,ESP
  0086c5b3  83 EC 50       SUB ESP,0x50
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0086c5b0
string literals (0):
direct callees (5):
  0x00710980  (unnamed)
  0x00946714  (unnamed)
  0x00710CA0  (unnamed)
  0x0085FEC0  CSWCAnimBase::ReplaceTexture_2
  0x009196EE  __security_check_cookie
struct offsets touched (top 5):
  +0x178 [dec 376]  x3  via ECX,EDX
  +0x17C [dec 380]  x3  via EAX,EDX
  +0x34 [dec 52]  x2  via EDX
  +0x138 [dec 312]  x1  via EAX
  +0x14C [dec 332]  x1  via EAX
```

### Static facts (script output, report-only)
- size 323 B, RET 12
- callers 2: 0x00868230, 0x0086b210
- callees: 0x00710980 (CALL), 0x00946714 (CALL), 0x00710ca0 (CALL), 0x0085fe10 (CALL), 0x009196ee __security_check_cookie (CALL)
- indirect: 0x0086c4ce slot 0x34, 0x0086c554 slot 0x34
- twins: none

## Card B17-3-16

GOG function: MdlNodeLight::InternalParseField  gog 0x008c7500  callconv=(blank)  param_size_bytes=4  gog_size=2293  gog_retn=4
Candidate Steam address: 0x004878b0 (size 15890, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.51) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00486fb0:STR26/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008c6ff0 MdlNode::InternalParseField  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008c7e00 MdlNodeEmitter::InternalParseField -> 0x004878b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (12), mapped to Steam through trusted rows only:
  0x0081b7a0 Global::Parse -> Steam ?  in candidate callees: n/a
  0x008c5260 Global::AurMessageGetToken -> Steam ?  in candidate callees: n/a
  0x008c5350 Global::AurMessageParseFloat -> Steam ?  in candidate callees: n/a
  0x008c5370 Global::AurMessageParseIntOrBool -> Steam ?  in candidate callees: n/a
  0x008c5f90 Global::Parse_11 -> Steam ?  in candidate callees: n/a
  0x008c6720 Global::AddGenericControllerSingle -> Steam ?  in candidate callees: n/a
  0x008c6c50 Global::AddGenericController -> Steam ?  in candidate callees: n/a
  0x008c6e50 Global::AddBezierController -> Steam ?  in candidate callees: n/a
  ... 4 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (28): 'fadingLight'; 'flareradius'; 'flarecolorshifts'; 'flarepositions' ...
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 20 common): GOG {0x4*,0x8*,0xc,0x18*,0x38*,0x44*,0x4c,0x50*,0x58*,0x60*,0x64*,0x6c*,0x78*,0x84*,...(+8)} | Steam {0x2,0x4*,0x8*,0x18*,0x24,0x38*,0x44*,0x50*,0x54,0x58*,0x5c,0x60*,0x64*,0x68,...(+44)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x004878b0: 16-byte aligned: YES
  004878b0  55             PUSH EBP
  004878b1  8B EC          MOV EBP,ESP
  004878b3  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004878b0
string literals (336):
  0x0098D304  at 0048794d  'p2p_sel'
  0x0098D304  at 00487962  'p2p_sel'
  0x0098D2F4  at 00487988  'affectedByWind'
  0x0098D2F4  at 0048799d  'affectedByWind'
  0x0098D2E8  at 004879c3  'm_isTinted'
  0x0098D2E8  at 004879d8  'm_isTinted'
  0x0098D2E0  at 004879fe  'bounce'
  0x0098D2E0  at 00487a13  'bounce'
  0x00988318  at 00487a39  'random'
  0x00988318  at 00487a4e  'random'
  0x0098D2D8  at 00487a74  'inherit'
  0x0098D2D8  at 00487a89  'inherit'
direct callees (25):
  0x00484D10  Global::AurMessageGetToken
  0x009370C7  _stricmp
  0x00919F10  _strlen
  0x00484F10  (unnamed)
  0x00484E20  (unnamed)
  0x004E52D0  (unnamed)
  0x004E52F0  (unnamed)
  0x004E5310  (unnamed)
  0x004E5330  (unnamed)
  0x004E5350  (unnamed)
  0x004E5370  (unnamed)
  0x004E5390  (unnamed)
  0x004E53C0  (unnamed)
  0x004E53E0  (unnamed)
  0x004E5410  (unnamed)
  0x004E5440  (unnamed)
  0x004E5470  (unnamed)
  0x004E54A0  (unnamed)
  0x00484EE0  (unnamed)
  0x00484DC0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 004878b0 FUN_004878b0 size=15890 ret=4 callers=1 callees=25

void __thiscall FUN_004878b0(CFrameWnd *param_1,int param_2)

{
  char *pcVar1;
  int iVar2;
  size_t sVar3;
  CFrameWnd *pCVar4;
  CFrameWnd *pCVar5;
  int *piVar6;
  uint *puVar7;
  undefined4 uVar8;
  char local_24;
  char local_23;
  char local_22;
  char local_21;
  int local_20;
  char local_19;
  uint local_18;
  char local_12;
  char local_11;
  CFrameWnd *local_10;
  char local_b;
  char local_a;
  char local_9;
  int local_8;
  
  local_8 = 0;
  if (DAT_00a309bd == '\0') {
    local_11 = '\0';
    local_20 = 0;
    local_19 = '\0';
    local_b = '\0';
    local_21 = '\0';
    local_18 = 0;
    local_24 = '\0';
    local_12 = '\0';
    local_a = '\0';
    local_22 = '\0';
    local_23 = '\0';
    local_9 = '\0';
    pcVar1 = (char *)FUN_00484d10(param_2);
    iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("p2p",pcVar1);
    if (iVar2 == 0) {
      pcVar1 = &local_11;
      sVar3 = _strlen [GHIDRA-AUTO-LABEL: unreliable]("p2p");
      FUN_00484f10(sVar3 + param_2,pcVar1);
    }
    pcVar1 = (char *)FUN_00484d10(param_2);
    iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("p2p_sel",pcVar1);
    if (iVar2 == 0) {
      piVar6 = &local_20;
      sVar3 = _strlen [GHIDRA-AUTO-LABEL: unreliable]("p2p_sel");
      FUN_00484e20(sVar3 + param_2,piVar6);
    }
    pcVar1 = (char *)FUN_00484d10(param_2);
    iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("affectedByWind",pcVar1);
    if (iVar2 == 0) {
      pcVar1 = &local_19;
      sVar3 = _strlen [GHIDRA-AUTO-LABEL: unreliable]("affectedByWind");
      FUN_00484f10(sVar3 + param_2,pcVar1);
    }
    pcVar1 = (char *)FUN_00484d10(param_2);
    iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("m_isTinted",pcVar1);
    if (iVar2 == 0) {
      pcVar1 = &local_b;
      sVar3 = _strlen [GHIDRA-AUTO-LABEL: unreliable]("m_isTinted");
      FUN_00484f10(sVar3 + param_2,pcVar1);
    }
    pcVar1 = (char *)FUN_00484d10(param_2);
    iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("bounce",pcVar1);
    if (iVar2 == 0) {
      pcVar1 = &local_21;
      sVar3 = _strlen [GHIDRA-AUTO-LABEL: unreliable]("bounce");
      FUN_00484f10(sVar3 + param_2,pcVar1);
    }
    pcVar1 = (char *)FUN_00484d10(param_2);
    iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("random",pcVar1);
    if (iVar2 == 0) {
      puVar7 = &local_18;
/* ... 1991 more lines truncated by --max-lines */
```

### decoy: 0x00486fb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00486fb0: 16-byte aligned: YES
  00486fb0  55             PUSH EBP
  00486fb1  8B EC          MOV EBP,ESP
  00486fb3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00486fb0
string literals (56):
  0x0098C8A0  at 00486fdc  'color'
  0x0098C8A0  at 00487004  'color'
  0x0098C894  at 0048703c  'colorkey'
  0x0098C894  at 00487064  'colorkey'
  0x0098C884  at 00487099  'colorbezierkey'
  0x0098C884  at 004870c1  'colorbezierkey'
  0x0098C87C  at 004870f4  'radius'
  0x0098C87C  at 0048711c  'radius'
  0x0098C870  at 00487154  'radiuskey'
  0x0098C870  at 0048717c  'radiuskey'
  0x0098C860  at 004871b1  'radiusbezierkey'
  0x0098C860  at 004871d9  'radiusbezierkey'
direct callees (12):
  0x00484D10  Global::AurMessageGetToken
  0x009370C7  _stricmp
  0x00919F10  _strlen
  0x004861D0  (unnamed)
  0x00486700  (unnamed)
  0x00486900  (unnamed)
  0x00484E20  (unnamed)
  0x00485A40  (unnamed)
  0x0048E8E0  (unnamed)
  0x00429010  (unnamed)
  0x00484E00  (unnamed)
  0x00486AA0  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 15890 B, RET 4
- callers 1: 0x00453b50
- callees: 0x00484d10 (CALL), 0x009370c7 _stricmp (CALL), 0x00919f10 _strlen (CALL), 0x00484f10 (CALL), 0x00484e20 (CALL), 0x004e52d0 (CALL), ...
- strings: 0x0098c8a8 "tangentlengthbezierkey", 0x0098c8c0 "tangentlengthkey", 0x0098c8d4 "tangentlength", ...
- globals: 0x00a309bd READ
- twins: none

## Card B17-3-17

GOG function: CSWPlayerControlCamRelative::Control  gog 0x004f0950  callconv=(blank)  param_size_bytes=4  gog_size=8823  gog_retn=4
Candidate Steam address: 0x00865830 (size 8823, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.97) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 5 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 5 (callees: GetLastTarget 0x740660, ResetCameraToFollow 0x7672f0, SetPrecalculatedPosition 0x775aa0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00865690:STR0/CALLEE0/SIG0.088/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004f07b0 CSWPlayerControlCamRelative::Init  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004f2eb0 CSWPlayerControlCamRelative::GetMaxSpeed  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (34), mapped to Steam through trusted rows only:
  0x0040ca10 CClientExoApp::GetCreatureByGameObjectID -> Steam 0x0073f550  in candidate callees: yes
  0x0040ccf0 CClientExoApp::GetClientOptions -> Steam 0x0072fb00  in candidate callees: yes
  0x0040db20 CClientExoApp::GetLastTarget -> Steam 0x00740660  in candidate callees: yes
  0x00412fc0 CSWCCreature::ResetCameraToFollow -> Steam 0x007672f0  in candidate callees: yes
  0x00413280 Vector::operator+ -> Steam ?  in candidate callees: n/a
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x00413780 Vector::Dot -> Steam ?  in candidate callees: n/a
  0x00419020 Vector::operator* -> Steam ?  in candidate callees: n/a
  ... 26 more GOG callees
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 24 common): GOG {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x24*,0x30*,0x34*,0x38*,0x3c*,0x40*,0x5c*,0x60*,...(+10)} | Steam {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x24*,0x30*,0x34*,0x38*,0x3c*,0x40*,0x5c*,0x60*,...(+10)}
imm sets (* = in both, 23 common): GOG {0x2*,0x4*,0x5*,0x6*,0x8*,0xa*,0xc*,0x10*,0x1c*,0x24*,0x30*,0x41*,0x44*,0x50*,...(+9)} | Steam {0x2*,0x4*,0x5*,0x6*,0x8*,0xa*,0xc*,0x10*,0x1c*,0x24*,0x30*,0x41*,0x44*,0x50*,...(+9)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00865830: 16-byte aligned: YES
  00865830  55             PUSH EBP
  00865831  8B EC          MOV EBP,ESP
  00865833  81 EC 84 0A 00 00 SUB ESP,0xa84
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00865830
string literals (0):
direct callees (34):
  0x0073F550  CClientExoApp::GetCreatureByGameObjectID
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x0072FB00  CClientExoApp::GetClientOptions
  0x008E28E0  (unnamed)
  0x00515E60  (unnamed)
  0x00515A70  (unnamed)
  0x00515990  (unnamed)
  0x005158B0  (unnamed)
  0x00515620  (unnamed)
  0x00463120  (unnamed)
  0x00543F10  CSWSObject::SetOrientation
  0x00514340  (unnamed)
  0x00515D50  (unnamed)
  0x0091EBAD  (unnamed)
  0x005142F0  (unnamed)
  0x00867B40  (unnamed)
  0x005143C0  (unnamed)
  0x0044C8A0  Vector::operator+ (Vector3 add)
  0x00436200  (unnamed)
  0x00867AB0  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x58  via EAX,ECX,EDX
  +0x8 [dec 8]  x57  via EAX,ECX,EDX
  +0xC [dec 12]  x14  via EAX,ECX,EDX
  +0x10 [dec 16]  x7  via EAX,ECX,EDX
  +0x14 [dec 20]  x6  via EAX,ECX,EDX
  +0x40 [dec 64]  x6  via EAX,ECX,EDX
  +0x88 [dec 136]  x4  via EAX,EDX
  +0x5C [dec 92]  x3  via EAX,ECX,EDX
  +0x60 [dec 96]  x3  via EAX,ECX,EDX
  +0x38 [dec 56]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00865830 FUN_00865830 size=8823 ret=4 callers=0 callees=34

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_00865830(int param_1,float param_2)

{
  short sVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 *puVar4;
  float *pfVar5;
  int *piVar6;
  undefined4 uVar7;
  float10 fVar8;
  float10 fVar9;
  double dVar10;
  float fVar11;
  float local_a54;
  undefined1 local_9a8 [12];
  undefined1 local_99c [12];
  int local_990;
  undefined1 local_980 [12];
  undefined1 local_974 [12];
  int local_968;
  undefined1 local_958 [12];
  undefined1 local_94c [12];
  int local_940;
  int local_93c;
  undefined1 local_92c [12];
  undefined1 local_920 [248];
  undefined1 local_828 [12];
  undefined1 local_81c [12];
  undefined4 *local_810;
  undefined4 *local_80c;
  int local_7dc;
  undefined1 local_7cc [12];
  undefined1 local_7c0 [24];
  undefined1 local_7a8 [12];
  undefined1 local_79c [12];
  undefined4 *local_790;
  undefined4 *local_78c;
  int local_75c;
  undefined1 local_74c [12];
  undefined1 local_740 [24];
  undefined1 local_728 [12];
  undefined1 local_71c [24];
  undefined1 local_704 [12];
  undefined1 local_6f8 [12];
  undefined4 *local_6ec;
  undefined4 *local_6e8;
  int local_6b8;
  undefined1 local_6a8 [12];
  undefined1 local_69c [24];
  undefined1 local_684 [12];
  undefined1 local_678 [24];
  undefined1 local_660 [12];
  undefined1 local_654 [12];
  undefined4 *local_648;
  undefined4 *local_644;
  undefined4 *local_5fc;
  undefined4 *local_5f8;
  undefined4 *local_5f4;
  undefined4 *local_5f0;
  undefined4 *local_5ec;
  float local_5e8;
  float local_5e4;
  undefined1 local_5e0 [14];
  ushort local_5d2;
  float *local_5d0;
  float *local_5cc;
  float local_5c8;
  float local_5c4;
  float local_5c0;
  float local_5bc;
  float local_5b8;
  float local_5b4;
  float local_5b0;
  float local_5ac;
  float local_5a8;
  float local_5a4;
/* ... 863 more lines truncated by --max-lines */
```

### decoy: 0x00865690 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00865690: 16-byte aligned: YES
  00865690  55             PUSH EBP
  00865691  8B EC          MOV EBP,ESP
  00865693  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00865690
string literals (2):
  0x009A4854  at 00865708  'MaxTurnRate'
  0x009A4848  at 00865742  'MinTurnRate'
direct callees (8):
  0x0073F430  CClientExoApp::GetModule
  0x00733570  CExoString::CExoString_char
  0x0071B670  (unnamed)
  0x00733780  CExoString::~CExoString
  0x0072FB00  CClientExoApp::GetClientOptions
  0x007BC030  (unnamed)
  0x007BC0B0  (unnamed)
  0x007BC130  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x5  via ECX,EDX
  +0x8 [dec 8]  x1  via EDX
  +0x34 [dec 52]  x1  via EAX
  +0x48 [dec 72]  x1  via EAX
  +0x78 [dec 120]  x1  via EAX
  +0xB8 [dec 184]  x1  via EDX
  +0xC0 [dec 192]  x1  via EAX
  +0xC4 [dec 196]  x1  via EAX
  +0xC8 [dec 200]  x1  via EAX
  +0x130 [dec 304]  x1  via ECX
```

### Static facts (script output, report-only)
- size 8823 B, RET 4
- callees: 0x0073f550 CClientExoApp::GetCreatureByGameObjectID (CALL), 0x0077d800 (CALL), 0x0072fb00 CClientExoApp::GetClientOptions (CALL), 0x008e28e0 (CALL), 0x00515e60 (CALL), 0x00515a70 (CALL), ...
- indirect: 0x0086590c slot 0x94, 0x00865932 slot 0x9c, 0x00865aca slot 0x88, 0x00865b97 slot 0x24, 0x008663b1 slot 0x88, ...
- twins: none

## Card B17-3-18

GOG function: Global::QueryTimer  gog 0x00713490  callconv=(blank)  param_size_bytes=0  gog_size=158  gog_retn=0
Candidate Steam address: 0x00650de0 (size 26, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00650d40:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00713330 CSWRoomSurfaceMesh::CheckAABBAll  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00713530 CSWCollisionMesh::not_GetWalkableMaterialMask -> 0x00650de0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00713530 CSWCollisionMesh::not_GetWalkableMaterialMask -> Steam ?  in candidate callees: n/a
  0x00927050 unnamed -> Steam ?  in candidate callees: n/a
  0x00929ca0 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x8*,0xf4240} | Steam {0x8*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00650de0: 16-byte aligned: YES
  00650de0  55             PUSH EBP
  00650de1  8B EC          MOV EBP,ESP
  00650de3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00650de0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00650de0 FUN_00650de0 size=26 ret=0 callers=1 callees=1

LARGE_INTEGER FUN_00650de0(void)

{
  LARGE_INTEGER local_c;
  
  QueryPerformanceFrequency(&local_c);
  return (LARGE_INTEGER)local_c.QuadPart;
}
```

### decoy: 0x00650d40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00650d40: 16-byte aligned: YES
  00650d40  55             PUSH EBP
  00650d41  8B EC          MOV EBP,ESP
  00650d43  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00650d40
string literals (0):
direct callees (3):
  0x00650DE0  (unnamed)
  0x0091FC30  (unnamed)
  0x00920670  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 26 B, RET 0
- callers 1: 0x00650d40
- callees: 0x000000d6 (CALL)
- imports: QueryPerformanceFrequency (KERNEL32.DLL)
- twins: none

## Card B17-3-19

GOG function: CSWGuiLabel::`scalar  gog 0x00522830  callconv=(blank)  param_size_bytes=4  gog_size=138  gog_retn=4
Candidate Steam address: 0x007510b0 (size 138, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: rotation_in:0x00917a70:0.84>0.67
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00751140:STR0/CALLEE0/SIG0.1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00522390 CSWGuiMainInterface::InitializeMap -> 0x00750c10  trusted
- GOG-order neighbour after:  0x00522930 CSWGuiMainInterface::ClearOneAction -> 0x007511b0  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x004ce500 CSWGuiControl::Destructor -> Steam 0x00418640  in candidate callees: yes
  0x00913030 CSWGuiBorder::Destructor_2 -> Steam 0x00414c60  in candidate callees: yes
  0x00915660 CSWGuiText::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x4*,0x60*,0xd8*} | Steam {0x4*,0x60*,0xd8*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007510b0: 16-byte aligned: YES
  007510b0  55             PUSH EBP
  007510b1  8B EC          MOV EBP,ESP
  007510b3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007510b0
string literals (0):
direct callees (4):
  0x00417290  (unnamed)
  0x00414C60  CSWGuiBorder::Destructor_2
  0x00418640  CSWGuiControl::Destructor
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007510b0 FUN_007510b0 size=138 ret=4 callers=0 callees=4

undefined4 * __thiscall FUN_007510b0(undefined4 *param_1,uint param_2)

{
  uint uVar1;
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  puStack_c = &LAB_00948a53;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *param_1 = CSWGuiLabel::vftable;
  local_8 = 1;
  FUN_00417290(uVar1);
  local_8 = local_8 & 0xffffff00;
  FUN_00414c60();
  local_8 = 0xffffffff;
  FUN_00418640();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x00751140 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00751140: 16-byte aligned: YES
  00751140  55             PUSH EBP
  00751141  8B EC          MOV EBP,ESP
  00751143  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00751140
string literals (0):
direct callees (2):
  0x004142E0  (unnamed)
  0x0049B840  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 138 B, RET 4
- callees: 0x00417290 (CALL), 0x00414c60 CSWGuiBorder::Destructor_2 (CALL), 0x00418640 CSWGuiControl::Destructor (CALL), 0x009196fd operator delete (CALL)
- twins: none

## Card B17-3-20

GOG function: Global::QueryTimer  gog 0x00713490  callconv=(blank)  param_size_bytes=0  gog_size=158  gog_retn=0
Candidate Steam address: 0x00650640 (size 979, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.06 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006502f0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00713330 CSWRoomSurfaceMesh::CheckAABBAll  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00713530 CSWCollisionMesh::not_GetWalkableMaterialMask -> 0x00650de0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00713530 CSWCollisionMesh::not_GetWalkableMaterialMask -> Steam ?  in candidate callees: n/a
  0x00927050 unnamed -> Steam ?  in candidate callees: n/a
  0x00929ca0 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0xc,0x5c,0x60,0x88}
imm sets (* = in both, 1 common): GOG {0x8*,0xf4240} | Steam {0x2,0x3,0x4,0x8*,0x80,0x8c,0x9c}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00650640: 16-byte aligned: YES
  00650640  55             PUSH EBP
  00650641  8B EC          MOV EBP,ESP
  00650643  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00650640
string literals (0):
direct callees (6):
  0x0084A640  (unnamed)
  0x00919723  operator new
  0x0083EA60  CExoArrayList<uint32>::Add
  0x006536F0  (unnamed)
  0x009196FD  operator delete
  0x0064DDB0  Free(ptr*) delete-and-zero helper
struct offsets touched (top 5):
  +0x60 [dec 96]  x4  via ECX,EDX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0xC [dec 12]  x2  via EAX,EDX
  +0x5C [dec 92]  x2  via EAX,EDX
  +0x88 [dec 136]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00650640 FUN_00650640 size=979 ret=0 callers=1 callees=6

void __fastcall FUN_00650640(int param_1)

{
  int iVar1;
  undefined4 *puVar2;
  bool bVar3;
  uint uVar4;
  int iVar5;
  int local_90;
  int local_8c;
  int *local_88;
  int local_48;
  int local_38;
  int local_34;
  int local_2c;
  int local_24;
  int local_20;
  uint local_1c;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00958bf0;
  local_10 = ExceptionList;
  uVar4 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  if (*(int *)(param_1 + 0x5c) != 0) {
    FUN_0084a640(0);
    local_8 = 0;
    FUN_0084a640(0);
    local_8 = CONCAT31(local_8._1_3_,1);
    for (local_14 = 0; local_14 < *(int *)(param_1 + 0x5c) * 3; local_14 = local_14 + 1) {
      if (*(int *)(*(int *)(param_1 + 0x88) + local_14 * 4) == -1) {
        local_88 = (int *)FUN_00919723(8,uVar4);
        if (local_88 == (int *)0x0) {
          local_88 = (int *)0x0;
        }
        else {
          *local_88 = local_14;
          local_88[1 = byte +0x4] = -1;
        }
        FUN_0083ea60(local_88);
      }
    }
    iVar5 = FUN_00919723(-(uint)((int)((ulonglong)local_1c * 4 >> 0x20) != 0) |
                         (uint)((ulonglong)local_1c * 4),uVar4);
    for (local_14 = 0; local_14 < (int)local_1c; local_14 = local_14 + 1) {
      *(undefined4 *)(iVar5 + local_14 * 4) = **(undefined4 **)(local_20 + local_14 * 4);
    }
    local_24 = 0;
    while (local_24 < (int)local_1c) {
      for (local_2c = 0; *(int *)(iVar5 + local_2c * 4) == -1; local_2c = local_2c + 1) {
      }
      FUN_0083ea60(*(undefined4 *)(local_20 + local_2c * 4));
      local_24 = local_24 + 1;
      *(undefined4 *)(iVar5 + local_2c * 4) = 0xffffffff;
      local_48 = **(int **)(local_20 + local_2c * 4);
      iVar1 = *(int *)(*(int *)(param_1 + 0x60) + local_48 * 4);
      bVar3 = false;
LAB_00650800:
      if (!bVar3) {
        if (local_48 % 3 == 2) {
          local_8c = local_48 + -2;
        }
        else {
          local_8c = local_48 + 1;
        }
        for (local_14 = 0; local_14 < (int)local_1c; local_14 = local_14 + 1) {
          if ((*(int *)(iVar5 + local_14 * 4) != -1) &&
             (*(int *)(*(int *)(param_1 + 0x60) + local_8c * 4) ==
              *(int *)(*(int *)(param_1 + 0x60) + **(int **)(local_20 + local_14 * 4) * 4))) {
            FUN_0083ea60(*(undefined4 *)(local_20 + local_14 * 4));
            local_48 = **(int **)(local_20 + local_14 * 4);
            *(undefined4 *)(iVar5 + local_14 * 4) = 0xffffffff;
            local_24 = local_24 + 1;
            if (local_48 % 3 == 2) {
              local_90 = local_48 + -2;
/* ... 32 more lines truncated by --max-lines */
```

### decoy: 0x006502f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006502f0: 16-byte aligned: YES
  006502f0  55             PUSH EBP
  006502f1  8B EC          MOV EBP,ESP
  006502f3  83 EC 38       SUB ESP,0x38
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006502f0
string literals (0):
direct callees (2):
  0x00919723  operator new
  0x009196FD  operator delete
struct offsets touched (top 3):
  +0x5C [dec 92]  x6  via EAX,ECX,EDX
  +0x88 [dec 136]  x6  via EAX,ECX
  +0x60 [dec 96]  x3  via EAX,ECX
```

### Static facts (script output, report-only)
- size 979 B, RET 0
- callers 1: 0x0064eb00
- callees: 0x0084a640 (CALL), 0x00919723 operator new (CALL), 0x0083ea60 CExoArrayList<uint32>::Add (CALL), 0x006536f0 (CALL), 0x009196fd operator delete (CALL), 0x0064ddb0 (CALL)
- twins: none

## Card B17-3-21

GOG function: CSWGuiOptionsGraphics::Constructor  gog 0x00569ef0  callconv=(blank)  param_size_bytes=8  gog_size=3078  gog_retn=8
Candidate Steam address: 0x007d4620 (size 3078, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 10 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,CALLEE,SIG  (content level C2)
- evidence summary: STR 'BTN_ADVANCED'@009a19fc; 'CB_FORCESPEED'@009a1a0c; 'CB_GRASS'@009a1a1c; CALLEE 2 (callees: SetActiveControl 0x40ec00, SetDefaultColor 0x417140); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007d4460:STR0/CALLEE0/SIG0.056/SRV0/VT0
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
disp sets (* = in both, 18 common): GOG {0x4*,0x8*,0x48*,0x98*,0xc4*,0x288*,0x374*,0x8d5*,0x8e0*,0x950*,0xb20*,0xcf0*,0xec0*,0x1188*,...(+4)} | Steam {0x4*,0x8*,0x48*,0x98*,0xc4*,0x288*,0x374*,0x8d5*,0x8e0*,0x950*,0xb20*,0xcf0*,0xec0*,0x1188*,...(+4)}
imm sets (* = in both, 72 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+58)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+58)}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x007d4620: 16-byte aligned: YES
  007d4620  55             PUSH EBP
  007d4621  8B EC          MOV EBP,ESP
  007d4623  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007d4620
string literals (17):
  0x009A1A84  at 007d4801  'optgraphics_p'
  0x009A1A78  at 007d4821  'SLI_GAMMA'
  0x009A1818  at 007d485b  'LBL_TITLE'
  0x009A1810  at 007d4898  'LB_DESC'
  0x009A1A6C  at 007d48d4  'LBL_GAMMA'
  0x009A1A5C  at 007d4911  'BTN_RESOLUTION'
  0x009A1A48  at 007d494e  'BTN_RESOLUTIONLEFT'
  0x009A1A34  at 007d498a  'BTN_RESOLUTIONRIGHT'
  0x009A1A28  at 007d49c7  'CB_SHADOWS'
  0x009A1A1C  at 007d4a04  'CB_GRASS'
  0x009A1A0C  at 007d4a40  'CB_FORCESPEED'
  0x009A19FC  at 007d4acb  'BTN_ADVANCED'
direct callees (22):
  0x0040E5A0  CSWGuiPanel::Constructor
  0x0041AB60  (unnamed)
  0x00419740  LabelCtor
  0x0041CE90  (unnamed)
  0x0041A510  ButtonCtor
  0x0041A330  (unnamed)
  0x0091D67E  (unnamed)
  0x00710840  (unnamed)
  0x0040EE40  CSWGuiPanel::StartLoadFromLayout
  0x00733570  CExoString::CExoString_char
  0x0040F620  CSWGuiPanel::InitControl
  0x00733780  CExoString::~CExoString
  0x00753C40  (unnamed)
  0x00416E30  CSWGuiTextParams::SetText
  0x0040F5A0  CSWGuiPanel::StopLoadFromLayout
  0x007D2360  (unnamed)
  0x0041B1D0  (unnamed)
  0x00418AF0  CSWGuiControl::AddEvent
  0x007D6030  (unnamed)
  0x00760EA0  (unnamed)
struct offsets touched (top 10):
  +0x48 [dec 72]  x14  via ECX,EDX
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x98 [dec 152]  x1  via EAX
  +0xC4 [dec 196]  x1  via EDX
  +0x288 [dec 648]  x1  via EAX
  +0x374 [dec 884]  x1  via EDX
  +0x8D5 [dec 2261]  x1  via ECX
  +0x8E0 [dec 2272]  x1  via EAX
  +0x950 [dec 2384]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007d4620 FUN_007d4620 size=3078 ret=8 callers=3 callees=22

undefined4 * __thiscall FUN_007d4620(undefined4 *param_1,undefined4 param_2,int param_3)

{
  uint uVar1;
  undefined1 local_b8 [8];
  undefined1 local_b0 [8];
  undefined1 local_a8 [8];
  undefined1 local_a0 [8];
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
  undefined4 local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0096a3e9;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_0040e5a0(param_2);
  local_8 = 0;
  *param_1 = CSWGuiOptionsGraphics::vftable;
  FUN_0041ab60(uVar1);
  local_8._0_1_ = 1;
  FUN_00419740();
  local_8._0_1_ = 2;
  FUN_00419740();
  local_8._0_1_ = 3;
  FUN_00419740();
  local_8._0_1_ = 4;
  FUN_0041ce90();
  local_8._0_1_ = 5;
  FUN_0041a510();
  local_8._0_1_ = 6;
  FUN_0041a510();
  local_8._0_1_ = 7;
  FUN_0041a510();
  local_8._0_1_ = 8;
  FUN_0041a330();
  param_1[0x399 = byte +0xe64] = CSWGuiOptionsCheckbox::vftable;
  local_8._0_1_ = 9;
  FUN_0041a330();
  param_1[1099 = byte +0x112c] = CSWGuiOptionsCheckbox::vftable;
  local_8._0_1_ = 10;
  FUN_0041a330();
  param_1[0x4fd = byte +0x13f4] = CSWGuiOptionsCheckbox::vftable;
  local_8._0_1_ = 0xb;
  FUN_0041a510();
  local_8._0_1_ = 0xc;
  FUN_0041a510();
  local_8._0_1_ = 0xd;
  FUN_0041a510();
  local_8._0_1_ = 0xe;
  _eh_vector_constructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable]
            (param_1 + 0x70b,0x148,4,FUN_00419740,
             CMFCToolBarButtonsListButton::~CMFCToolBarButtonsListButton);
  local_8._0_1_ = 0xf;
  FUN_00710840("optgraphics_p");
  FUN_0040ee40(local_28,1,1);
  FUN_00733570("SLI_GAMMA");
  local_8._0_1_ = 0x10;
  FUN_0040f620(param_1 + 0x1a,local_30,1,1);
  local_8._0_1_ = 0xf;
  FUN_00733780();
/* ... 145 more lines truncated by --max-lines */
```

### decoy: 0x007d4460 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007d4460: 16-byte aligned: YES
  007d4460  55             PUSH EBP
  007d4461  8B EC          MOV EBP,ESP
  007d4463  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007d4460
string literals (0):
direct callees (8):
  0x00416EA0  CSWGuiTextParams::SetStrRef
  0x007335F0  CExoString::CExoString_copy
  0x0041D290  CSWGuiListBox::ClearItems
  0x00919F10  _strlen
  0x004178B0  (unnamed)
  0x0041FD90  CSWGuiListBox::AddControls
  0x0041E9A0  (unnamed)
  0x00733780  CExoString::~CExoString
struct offsets touched (top 4):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x2  via EDX
  +0xC [dec 12]  x1  via EDX
  +0x358 [dec 856]  x1  via EDX
```

### Static facts (script output, report-only)
- size 3078 B, RET 8
- callers 3: 0x007d2f70, 0x007d9280, 0x008a1df0
- callees: 0x0040e5a0 CSWGuiPanel::Constructor (CALL), 0x0041ab60 (CALL), 0x00419740 (CALL), 0x0041ce90 (CALL), 0x0041a510 (CALL), 0x0041a330 (CALL), ...
- indirect: 0x007d4d9e slot 0x98
- strings: 0x009a19fc "BTN_ADVANCED", 0x009a1a0c "CB_FORCESPEED", 0x009a1a1c "CB_GRASS", ...
- twins: none

## Card B17-3-22

GOG function: CAurBehaviorCameraFollow::Control  gog 0x008c4b30  callconv=(blank)  param_size_bytes=4  gog_size=1703  gog_retn=4
Candidate Steam address: 0x00506450 (size 55, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.16 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00505da0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008c4700 CAurBehaviorCameraFollow::Message  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008c51e0 CAurBehaviorCameraFollow::GetTargetWorldPosition  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x00413280 Vector::operator+ -> Steam ?  in candidate callees: n/a
  0x00419020 Vector::operator* -> Steam ?  in candidate callees: n/a
  0x0042a8f0 Quaternion::GetZAxis -> Steam ?  in candidate callees: n/a
  0x0080d120 Global::normalize_2 -> Steam ?  in candidate callees: n/a
  0x0080d220 Global::slerp -> Steam ?  in candidate callees: n/a
  0x0080d4b0 Global::YawPitchRoll -> Steam ?  in candidate callees: n/a
  0x008c31a0 CAurBehavior::Control -> Steam ?  in candidate callees: n/a
  0x00927220 Global::pow -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0xc,0x10,0x14*,0x30,0x34,0x38,0x3c,0x40,0x44,0x48,0x4c,0x50,...(+8)} | Steam {0x4*,0x8*,0x14*}
imm sets (* = in both, 2 common): GOG {0x4*,0x5,0x8*,0xc,0x10,0x18,0x24,0x41,0x44,0x198} | Steam {0x4*,0x8*,0xa4}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00506450: 16-byte aligned: YES
  00506450  55             PUSH EBP
  00506451  8B EC          MOV EBP,ESP
  00506453  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00506450
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x14 [dec 20]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00506450 FUN_00506450 size=55 ret=4 callers=1 callees=0

undefined4 * __thiscall FUN_00506450(int param_1,undefined4 *param_2)

{
  int iVar1;
  
  iVar1 = *(int *)(param_1 + 0x14);
  *param_2 = *(undefined4 *)(iVar1 + 0xa4);
  param_2[1 = byte +0x4] = *(undefined4 *)(iVar1 + 0xa8);
  param_2[2 = byte +0x8] = *(undefined4 *)(iVar1 + 0xac);
  return param_2;
}
```

### decoy: 0x00505da0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00505da0: 16-byte aligned: YES
  00505da0  55             PUSH EBP
  00505da1  8B EC          MOV EBP,ESP
  00505da3  81 EC 98 01 00 00 SUB ESP,0x198
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00505da0
string literals (0):
direct callees (8):
  0x005043D0  (unnamed)
  0x0091E390  (unnamed)
  0x0044C8A0  Vector::operator+ (Vector3 add)
  0x00436200  (unnamed)
  0x00515620  (unnamed)
  0x00515390  (unnamed)
  0x00515290  (unnamed)
  0x00463120  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x12  via EAX,ECX,EDX
  +0x8 [dec 8]  x12  via EAX,ECX,EDX
  +0x10 [dec 16]  x10  via EAX,ECX,EDX
  +0x30 [dec 48]  x8  via EAX,ECX,EDX
  +0x3C [dec 60]  x8  via EAX,ECX,EDX
  +0x44 [dec 68]  x8  via EAX,ECX,EDX
  +0xC [dec 12]  x7  via EAX,ECX,EDX
  +0x60 [dec 96]  x5  via ECX,EDX
  +0x54 [dec 84]  x4  via EAX,ECX
  +0x68 [dec 104]  x4  via EAX,ECX,EDX
```

### Static facts (script output, report-only)
- size 55 B, RET 4
- callers 1: 0x0049ae90
- twins: none
