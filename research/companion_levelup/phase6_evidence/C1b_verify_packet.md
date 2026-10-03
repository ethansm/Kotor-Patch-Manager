# C1b blind verification packet (R10 level-up UI identities, KOTOR II Steam/Aspyr swkotor2.exe)

You are a verifier (Haiku sieve, tier 1 of RE_AGENT_GUIDE.md). Each CARD has a CLAIM (an address with a proposed name, vftable
or hook-site description) and EVIDENCE for the address named in the claim. Roughly two thirds of the cards are deliberately
WRONG (wrong address, wrong name, wrong vftable, wrong exit, wrong callee). You are not told which. A wrong claim sits next to
evidence that contradicts it; look for the contradiction.

## Rules (from RE_AGENT_GUIDE.md sections 3 and 4)
1. Assume the claim is a NEIGHBOURING function/site with the same signature. You are FORBIDDEN to answer `match` on `ret N`,
   size, prologue or name alone. To answer `match` you MUST cite at least one CONTENT ANCHOR from the evidence shown: a string
   literal (address + text), a confirmed callee (address + name from the ANCHORS list), or a member offset / instruction
   with its asm line. If you cannot, answer `unsure`.
2. Do NOT do hex/decimal or stack math. TIER-0 FACTS are computed for you. Bare numbers in decompile are DECIMAL; the script
   has added hex and scaled byte offsets: `int *p; p + 100` = byte +0x190 (use the `{int*: byte +0x...}` note). Asm operands are always hex.
3. Thiscall thunks print without `__thiscall`; ECX passes through. Ghidra auto labels (FUN_*, FID_*) are not evidence.
4. A claimed NAME must be supported by what the code does, not by the claim's wording. If the code does the sibling thing
   (feats vs powers vs skills; ctor vs dtor; add-row vs sort; call to the feat test vs the spell test) answer `mismatch`.
5. For hook-site cards: check that the claimed address is an instruction START whose bytes equal the claimed bytes, that the
   register role is what the preceding instructions establish (quote them), and that the exit target is the claimed kind of
   instruction. A different address, bytes or exit than claimed = `mismatch`.
6. Absence is not proof. `unsure` is always allowed and is not penalised.

## Few-shot negatives
- Neighbour pair: 0x5644B0 and 0x564770 both were called AddMoveToPointAction with the same RET; 0x5644B0 calls AddActionToFront
  (005645e4 CALL 0x00540ca0), 0x564770 calls AddAction (005648a4 CALL 0x0053f7f0). The callee is the anchor.
- Bool arg + RET 4: `Stack Slot Match: TRUE (GOG 1 byte -> 1 slot; ASM RET 4)` is normal, not a mismatch.
- Decimal + scaled offset: `*(short *)(param_1 + 100)` with `int *param_1` is byte offset 0x190, asm `MOV word ptr [EDX + 0x190],AX`.

## Output: one JSON object per card, in this order (verdict LAST)
```json
{"card": "C000", "observed_prologue_or_bytes": "55 8b ec", "content_anchor": "PUSH 0x9965f8 'text' at 005cbcb5 | callee 0x008480B0 | asm line",
 "neighbour_decoy_considered": "<what else could this be, and why not>", "auto_label_ignored": true,
 "confidence_1_to_5": 4, "verdict": "match | mismatch | unsure", "reason": "<= 20 words"}
```
Known DB facts you may rely on: 0x8480B0 = CSWCLevelUpStats::CanChooseFeat, 0x847E50 = CalcNumberFeats, 0x847F90 =
ComputeNumberKnownSpellsLeft, 0x8489F0 = AddDefaultPackageFeats, 0x848AE0 = AddDefaultPackageSpells, 0x593450 = CExoArrayList<uint16>::Add,
0x733780 = CExoString::~CExoString, 0x73F550 = CClientExoApp::GetCreatureByGameObjectID.
Everything else in the CLAIMs is unverified. Answer every card; do not skip.


# BATCH 1 (cards C001-C022)

---
## CARD C001
**CLAIM:** Class `CSWGuiFeatsCharGen` has vftable 0x009AA434; its constructor is 0x00909E00.

```
ASM lines of ctor 0x00909E00 containing a 0x9AAxxx constant:
00909e4a  c70134a49a00          MOV dword ptr [ECX],0x9aa434
0090a0d2  6824a49a00            PUSH 0x9aa424
0090a12c  682ca39a00            PUSH 0x9aa32c
0090a169  6818a39a00            PUSH 0x9aa318
0090a1a5  6804a49a00            PUSH 0x9aa404
0090a1e2  68e4a39a00            PUSH 0x9aa3e4

ANCHORS of ctor:
anchors for 0x00909E00 (FUN_00909e00)
string literals (14):
  0x009AA424  at 0090a0d2  'FTCHRGEN_p'
  0x009A969C  at 0090a0f2  'MAIN_TITLE_LBL'
  0x009AA32C  at 0090a12c  'SUB_TITLE_LBL'
  0x009AA318  at 0090a169  'REMAINING_BOX_LBL'
  0x009AA404  at 0090a1a5  'STD_REMAINING_SELECTIONS_LBL'
  0x009AA3E4  at 0090a1e2  'STD_SELECTIONS_REMAINING_LBL'
  0x009A1810  at 0090a21f  'LB_DESC'
  0x009AA3D8  at 0090a268  'LB_FEATS'
  0x0099D72C  at 0090a2a4  'LBL_NAME'
  0x009A6968  at 0090a2e1  'BTN_ACCEPT'

VTABLE DUMP:
0x9AA434 (28 slots)
  slot 0 +0x0 = 0x90A970
  slot 15 +0x3C = 0x90B200
  slot 18 +0x48 = 0x90AD90
```

---
## CARD C002
**CLAIM:** Function at 0x008F4AF0 is `CSWGuiLevelUpCharGen::Constructor`: level-up outer panel ctor (+0x68 temp baseline creature, +0x2CB4 creature id).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x008F4AF0, size 4281, header ret=12 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x008F4AF0 (FUN_008f4af0)
string literals (38):
  0x009A96AC  at 008f4f27  'MAINCG_p'
  0x009A969C  at 008f4f47  'MAIN_TITLE_LBL'
  0x0099D72C  at 008f4f81  'LBL_NAME'
  0x009A9690  at 008f4fbe  'LBL_CLASS'
  0x009A9680  at 008f4ffb  'LBL_LEVEL_VAL'
  0x009A9678  at 008f5037  'STR_LBL'
  0x009A9670  at 008f5074  'DEX_LBL'
  0x009A9668  at 008f50b1  'CON_LBL'
  0x009A9660  at 008f50ed  'INT_LBL'
  0x009A9658  at 008f512a  'WIS_LBL'
  0x009A9650  at 008f5170  'CHA_LBL'
  0x009A9644  at 008f51b5  'STR_AB_LBL'
  0x009A9638  at 008f51fb  'DEX_AB_LBL'
  0x009A962C  at 008f5241  'CON_AB_LBL'
  0x009A9620  at 008f5286  'INT_AB_LBL'
  0x009A9614  at 008f52cc  'WIS_AB_LBL'
  0x009A9608  at 008f5312  'CHA_AB_LBL'
  0x009A9600  at 008f5357  'LBL_VIT'
  0x009A7504  at 008f539d  'LBL_DEF'
  0x009A95F0  at 008f53e3  'NEW_WILL_LBL'
  0x009A95E0  at 008f5428  'NEW_REFL_LBL'
  0x009A95D0  at 008f546e  'NEW_FORT_LBL'
  0x009A6E64  at 008f54b4  'LBL_BEVEL_L'
  0x009A6E58  at 008f54f9  'LBL_BEVEL_M'
  0x009A95C4  at 008f553f  'LBL_BEVEL_R'
  0x009A3C58  at 008f5585  'LBL_FORTITUDE'
  0x009A3C4C  at 008f55ca  'LBL_REFLEX'
  0x009A3C40  at 008f5610  'LBL_WILL'
  0x009A95B8  at 008f5656  'MODEL_LBL'
  0x009A95A8  at 008f569b  'PORTRAIT_LBL'
  0x009A9598  at 008f56e1  'LBL_STATSBACK'
  0x009A39DC  at 008f5727  'LBL_STATSBORDER'

DECOMPILE (offsets annotated; truncated):
// FN 008f4af0 FUN_008f4af0 size=4281 ret=12 callers=1 callees=26

undefined4 * __thiscall
FUN_008f4af0(undefined4 *param_1,undefined4 param_2,undefined4 param_3,int *param_4)

{
  uint uVar1;
  undefined4 uVar2;
  float10 fVar3;
  undefined4 local_1d4;
  undefined4 local_1d0;
  undefined4 local_1a8;
  undefined4 local_1a4;
  undefined4 local_1a0;
  undefined1 local_19c [8];
  undefined4 local_194;
  undefined4 local_190;
  undefined4 local_18c;
  undefined4 local_188;
  undefined4 local_184;
  undefined4 local_180;
  undefined4 local_17c;
  undefined1 local_178 [16];
  undefined1 local_168 [8];
  int local_160;
  undefined4 local_15c;
  undefined1 local_158 [8];
  undefined1 local_150 [8];
  undefined1 local_148 [8];
  undefined1 local_140 [8];
  undefined1 local_138 [8];
  undefined1 local_130 [8];
  undefined1 local_128 [8];
  undefined1 local_120 [8];
  undefined1 local_118 [8];
  undefined1 local_110 [8];
  undefined1 local_108 [8];
  undefined1 local_100 [8];
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
/* ... 365 more lines truncated by --max-lines */
```

---
## CARD C003
**CLAIM:** Function at 0x00902F60 is `CSWGuiLevelUpPanel::Constructor`: level-up step panel ctor (+0x68 = live client creature).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x00902F60, size 2867, header ret=8 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x00902F60 (FUN_00902f60)
string literals (5):
  0x009AA194  at 009030d8  'LEVELUPPNL_p'
  0x009AA18C  at 0090311b  'LBL_%d'
  0x009AA17C  at 0090317b  'BTN_STEPNAME%d'
  0x009AA170  at 009031bd  'LBL_NUM%d'
  0x0099E67C  at 00903239  'BTN_CANCEL'
direct callees (26):
  0x0041A510  (unnamed)
  0x00847E50  CSWCLevelUpStats::CalcNumberFeats
  0x004149F0  (unnamed)
  0x0091D67E  (unnamed)
  0x0084A640  (unnamed)
  0x008470A0  (unnamed)
  0x00760EA0  (unnamed)
  0x00733700  CExoString::Constructor_5
  0x00846200  (unnamed)
  0x00734270  CExoString::Format
  0x00847F90  CSWCLevelUpStats::ComputeNumberKnownSpellsLeft
  0x0040E5A0  CSWGuiPanel::Constructor
  0x0073F550  CClientExoApp::GetCreatureByGameObjectID
  0x00416E30  CSWGuiTextParams::SetText
  0x0040F5A0  CSWGuiPanel::StopLoadFromLayout
  0x00733780  CExoString::~CExoString
  0x00844860  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x00847A70  CSWCLevelUpStats::AddGrantedPowers
  0x0040F620  CSWGuiPanel::InitControl
  0x00418AF0  CSWGuiControl::AddEvent
  0x00733540  CExoString::Constructor
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x00710840  (unnamed)
  0x0073FEA0  CClientExoApp::GetSWGuiManager
  0x0040EE40  CSWGuiPanel::StartLoadFromLayout

DECOMPILE (offsets annotated; truncated):
// FN 00902f60 FUN_00902f60 size=2867 ret=8 callers=1 callees=26

/* WARNING: Removing unreachable block (ram,0x009032ab) */

undefined4 * __thiscall FUN_00902f60(undefined4 *param_1,undefined4 param_2,undefined4 param_3)

{
  uint uVar1;
  undefined4 uVar2;
  undefined1 local_90 [8];
  undefined1 local_88 [8];
  undefined1 local_80 [8];
  undefined1 local_78 [8];
  undefined1 local_70 [8];
  undefined1 local_68 [8];
  undefined1 local_60 [8];
  undefined1 local_58 [16];
  undefined1 local_48 [4];
  int local_44;
  undefined1 local_3c [4];
  int local_38;
  int local_30;
  byte local_29;
  uint local_28;
  undefined1 local_24 [8];
  int local_1c;
  uint local_18;
  byte local_11;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0097fc6b;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_0040e5a0(param_2);
  local_8 = 0;
  *param_1 = CSWGuiLevelUpPanel::vftable;
  _eh_vector_constructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable](param_1 + 0x1c,0x1c0,5,FUN_00758960,FUN_00420450);
  local_8._0_1_ = 1;
  _eh_vector_constructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable]
            (param_1 + 0x24c,0x148,5,FUN_00419740,
             CMFCToolBarButtonsListButton::~CMFCToolBarButtonsListButton);
  local_8._0_1_ = 2;
  _eh_vector_constructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable](param_1 + 0x3e6,0x1d0,5,FUN_0041a510,FUN_0041a3d0);
  local_8._0_1_ = 3;
  FUN_0041a510(uVar1);
/* ... 147 more lines truncated by --max-lines */
```

---
## CARD C004
**CLAIM:** Function at 0x00846690 is `CSWCLevelUpStats::GetSkillPointCost`: returns 1 for class skill, 2 cross-class (2 - classSkill).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x00846690, size 555, header ret=4 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x00846690 (FUN_00846690)
string literals (0):
direct callees (6):
  0x00844860  (unnamed)
  0x00844E40  CSWSCreatureStats::GetClassByPosition
  0x006F24D0  (unnamed)
  0x00845C10  (unnamed)
  0x00844FF0  (unnamed)
  0x006F2440  (unnamed)
struct offsets touched (top 7 of [REG + 0xNN], EBP/ESP excluded):
  +0xB6 [dec 182]  x6  via ECX,EDX
  +0x90 [dec 144]  x2  via EAX,EDX
  +0x123 [dec 291]  x2  via EAX,ECX
  +0x1C [dec 28]  x1  via EDX
  +0x32 [dec 50]  x1  via EDX
  +0x124 [dec 292]  x1  via ECX
  +0x12C [dec 300]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 00846690 FUN_00846690 size=555 ret=4 callers=2 callees=6

undefined4 __thiscall FUN_00846690(int param_1,ushort param_2)

{
  bool bVar1;
  byte bVar2;
  char cVar3;
  int iVar4;
  undefined4 uVar5;
  bool bVar6;
  char local_26;
  char local_24;
  byte local_d;
  uint local_c;
  
  bVar1 = false;
  iVar4 = (uint)param_2 * 0x20 + *(int *)(DAT_00a1b4d0 + 0x12c [dec 300]);
  if (iVar4 == 0) {
    uVar5 = 0;
  }
  else {
    bVar6 = (*(uint *)(iVar4 + 0x1c) >> 3 & 1) != 0;
    for (local_c = 0; (int)local_c < (int)(uint)*(byte *)(param_1 + 0x32); local_c = local_c + 1) {
      bVar2 = FUN_00844e40(local_c & 0xff);
      if (((uint)bVar2 * 0x238 + *(int *)(DAT_00a1b4d0 + 0x124) != 0) &&
         (iVar4 = FUN_006f24d0(param_2), iVar4 != 0)) {
        bVar6 = true;
        iVar4 = FUN_006f2440(param_2);
        if ((iVar4 != 0) || (iVar4 = FUN_00844ff0(param_2), iVar4 != 0)) {
          bVar1 = true;
          break;
        }
      }
    }
    if (bVar6) {
      if ((bVar1) || (1 < *(ushort *)(param_1 + 0xb6))) {
        if ((bVar1) && (*(short *)(param_1 + 0xb6) == 0)) {
          uVar5 = 0;
        }
        else {
          cVar3 = FUN_00844860();
          local_d = cVar3 + 3;
          if (!bVar1) {
            local_d = local_d / 2;
          }
          if (param_2 < *(byte *)(DAT_00a1b4d0 + 0x123)) {
            local_24 = *(char *)((uint)param_2 + *(int *)(param_1 + 0x90));
          }
/* ... 34 more lines truncated by --max-lines */
```

---
## CARD C005
**CLAIM:** Function at 0x0090AEA0 is `CSWGuiPowersLevelUp::BuildKnownPowerLists`: builds known-power vector (+0x1BC8) alphabetically, granted powers, picks left, then choosable 0x9098D0.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0090AEA0, size 861, header ret=0 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x0090AEA0 (FUN_0090aea0)
string literals (0):
direct callees (16):
  0x00847E50  CSWCLevelUpStats::CalcNumberFeats
  0x0073F750  ClientApp::GetGuiInGame
  0x0089B600  (unnamed)
  0x0090C230  (unnamed)
  0x0084A640  (unnamed)
  0x0064DC30  (unnamed)
  0x008470A0  (unnamed)
  0x0089CB60  (unnamed)
  0x0090BD50  (unnamed)
  0x008450E0  (unnamed)
  0x00593450  CExoArrayList<uint16>::Add
  0x0049B840  (unnamed)
  0x00416EA0  (unnamed)
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x0090B430  CSWGuiFeatsCharGen::OnAcceptButton
  0x00844560  (unnamed)
struct offsets touched (top 11 of [REG + 0xNN], EBP/ESP excluded):
  +0x1BE0 [dec 7136]  x10  via EAX,ECX,EDX
  +0x1BDA [dec 7130]  x4  via EAX,ECX
  +0x1BDB [dec 7131]  x4  via EAX,ECX,EDX
  +0x1BDE [dec 7134]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x68 [dec 104]  x1  via EAX
  +0xAC [dec 172]  x1  via EDX
  +0xBC [dec 188]  x1  via EDX
  +0x310 [dec 784]  x1  via ECX
  +0x1BDC [dec 7132]  x1  via EDX
  +0x1BDD [dec 7133]  x1  via EDX

DECOMPILE (offsets annotated; truncated):
// FN 0090aea0 FUN_0090aea0 size=861 ret=0 callers=1 callees=16

void __fastcall FUN_0090aea0(int param_1)

{
  short sVar1;
  undefined2 uVar2;
  int iVar3;
  char cVar4;
  int local_24;
  uint local_20;
  uint local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009810e8;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_14 = *(int *)(*(int *)(param_1 + 0x68) + 0x310);
  if (((*(uint *)(param_1 + 0x1be0) & 1) == 0) && ((*(uint *)(param_1 + 0x1be0) >> 1 & 1) == 0)) {
    FUN_00844560(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  }
  for (local_18 = 0; (int)local_18 < (int)(*(uint *)(local_14 + 0xbc) & 0xffff);
      local_18 = local_18 + 1) {
    sVar1 = FUN_008450e0(local_18 & 0xffff);
    if ((sVar1 != 0xcf) && (sVar1 = FUN_008450e0(local_18 & 0xffff), sVar1 != 0xee)) {
      uVar2 = FUN_008450e0(local_18 & 0xffff);
      FUN_00593450(uVar2);
    }
  }
  *(uint *)(param_1 + 0x1be0) = *(uint *)(param_1 + 0x1be0) & 0xfffffffb;
  if ((*(uint *)(param_1 + 0x1be0) >> 1 & 1) == 0) {
    FUN_0084a640(0);
    local_8 = 0;
    if ((*(uint *)(param_1 + 0x1be0) & 1) == 0) {
      FUN_008470a0(&local_24,1);
    }
    else {
      FUN_008470a0(&local_24,0);
    }
    if (0 < (int)local_20) {
      for (local_18 = 0; (int)local_18 < (int)local_20; local_18 = local_18 + 1) {
        FUN_00593450(*(undefined2 *)(local_24 + local_18 * 4));
      }
      while (local_18 = local_20 - 1, -1 < (int)local_18) {
        iVar3 = FUN_0064dc30(*(undefined2 *)(local_24 + local_18 * 4));
/* ... 47 more lines truncated by --max-lines */
```

---
## CARD C006
**CLAIM:** Function at 0x0090B6B0 is `CSWGuiFeatsCharGen::OnRecommendedButton`: feats Recommended button: AddDefaultPackageFeats path (ignores grid filter).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0090B6B0, size 472, header ret=0 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x0090B6B0 (FUN_0090b6b0)
string literals (0):
direct callees (13):
  0x0073F750  ClientApp::GetGuiInGame
  0x00840EF0  (unnamed)
  0x0084A640  (unnamed)
  0x00410530  CSWGuiManager::AddPanel
  0x0090C120  (unnamed)
  0x0089CB60  (unnamed)
  0x00844E40  CSWSCreatureStats::GetClassByPosition
  0x008489F0  CSWCLevelUpStats::AddDefaultPackageFeats
  0x0090BF90  (unnamed)
  0x0090BD50  (unnamed)
  0x005933B0  (unnamed)
  0x00416EA0  (unnamed)
  0x0064DDB0  Free(ptr*) delete-and-zero helper
struct offsets touched (top 9 of [REG + 0xNN], EBP/ESP excluded):
  +0x68 [dec 104]  x2  via EAX,ECX
  +0x310 [dec 784]  x2  via ECX,EDX
  +0x1BE0 [dec 7136]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via ECX
  +0x1C [dec 28]  x1  via EAX
  +0x32 [dec 50]  x1  via EAX
  +0xAC [dec 172]  x1  via EAX
  +0x1BCC [dec 7116]  x1  via ECX
  +0x1BD0 [dec 7120]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 0090b6b0 FUN_0090b6b0 size=472 ret=0 callers=1 callees=13

void FUN_0090b6b0(void)

{
  undefined1 uVar1;
  uint uVar2;
  int iVar3;
  int extraout_var;
  int iVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  int local_34[3 = byte +0xc];
  undefined4 local_28;
  int local_24;
  int local_20;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00981150;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_005933b0(0);
  local_8 = 0;
  iVar4 = extraout_var;
  FUN_0084a640(0);
  local_8 = CONCAT31(local_8._1_3_,1);
  local_14 = *(int *)(*(int *)(iVar4 + 0x68) + 0x310);
  uVar1 = FUN_00844e40(*(byte *)(local_14 + 0x32) - 1);
  FUN_008489f0(uVar1,&local_24,(*(uint *)(iVar4 + 0x1be0) >> 1 & 1) == 0,
               *(uint *)(iVar4 + 0x1be0) >> 1 & 1);
  while (*(int *)(iVar4 + 0x1bd0) != 0) {
    FUN_0090c120(**(undefined2 **)(iVar4 + 0x1bcc));
  }
  FUN_00840ef0(local_20);
  for (local_18 = 0; local_18 < local_20; local_18 = local_18 + 1) {
    FUN_0090bf90(*(undefined2 *)(local_24 + local_18 * 2));
    *(uint *)(local_34[0] + local_18 * 4) = (uint)*(ushort *)(local_24 + local_18 * 2);
  }
  uVar6 = *(undefined4 *)(DAT_00a1b4a4 + 4);
  iVar3 = FUN_0073f750();
  local_28 = *(undefined4 *)(iVar3 + 0xac);
  FUN_0089cb60(local_34,1);
  FUN_00416ea0(0xa510);
/* ... 10 more lines truncated by --max-lines */
```

---
## CARD C007
**CLAIM:** Function at 0x00846690 is `CSWCLevelUpStats::DecrementSkillRank`: skill rank-down; refunds 1 (class skill) or 2 (cross-class) points.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x00846690, size 555, header ret=4 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x00846690 (FUN_00846690)
string literals (0):
direct callees (6):
  0x00844860  (unnamed)
  0x00844E40  CSWSCreatureStats::GetClassByPosition
  0x006F24D0  (unnamed)
  0x00845C10  (unnamed)
  0x00844FF0  (unnamed)
  0x006F2440  (unnamed)
struct offsets touched (top 7 of [REG + 0xNN], EBP/ESP excluded):
  +0xB6 [dec 182]  x6  via ECX,EDX
  +0x90 [dec 144]  x2  via EAX,EDX
  +0x123 [dec 291]  x2  via EAX,ECX
  +0x1C [dec 28]  x1  via EDX
  +0x32 [dec 50]  x1  via EDX
  +0x124 [dec 292]  x1  via ECX
  +0x12C [dec 300]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 00846690 FUN_00846690 size=555 ret=4 callers=2 callees=6

undefined4 __thiscall FUN_00846690(int param_1,ushort param_2)

{
  bool bVar1;
  byte bVar2;
  char cVar3;
  int iVar4;
  undefined4 uVar5;
  bool bVar6;
  char local_26;
  char local_24;
  byte local_d;
  uint local_c;
  
  bVar1 = false;
  iVar4 = (uint)param_2 * 0x20 + *(int *)(DAT_00a1b4d0 + 0x12c [dec 300]);
  if (iVar4 == 0) {
    uVar5 = 0;
  }
  else {
    bVar6 = (*(uint *)(iVar4 + 0x1c) >> 3 & 1) != 0;
    for (local_c = 0; (int)local_c < (int)(uint)*(byte *)(param_1 + 0x32); local_c = local_c + 1) {
      bVar2 = FUN_00844e40(local_c & 0xff);
      if (((uint)bVar2 * 0x238 + *(int *)(DAT_00a1b4d0 + 0x124) != 0) &&
         (iVar4 = FUN_006f24d0(param_2), iVar4 != 0)) {
        bVar6 = true;
        iVar4 = FUN_006f2440(param_2);
        if ((iVar4 != 0) || (iVar4 = FUN_00844ff0(param_2), iVar4 != 0)) {
          bVar1 = true;
          break;
        }
      }
    }
    if (bVar6) {
      if ((bVar1) || (1 < *(ushort *)(param_1 + 0xb6))) {
        if ((bVar1) && (*(short *)(param_1 + 0xb6) == 0)) {
          uVar5 = 0;
        }
        else {
          cVar3 = FUN_00844860();
          local_d = cVar3 + 3;
          if (!bVar1) {
            local_d = local_d / 2;
          }
          if (param_2 < *(byte *)(DAT_00a1b4d0 + 0x123)) {
            local_24 = *(char *)((uint)param_2 + *(int *)(param_1 + 0x90));
          }
/* ... 34 more lines truncated by --max-lines */
```

---
## CARD C008
**CLAIM:** Function at 0x0090EDE0 is `CSWGuiSkillsCharGen::OnRankUpButton`: skills rank-up button handler: cost via 0x846350, rank change via 0x846690.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0090EDE0, size 2082, header ret=0 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x0090EDE0 (FUN_0090ede0)
string literals (0):
direct callees (20):
  0x00846350  (unnamed)
  0x007337C0  CExoString::operator=
  0x007335F0  CExoString::CExoString_copy
  0x0073F750  ClientApp::GetGuiInGame
  0x0075CA30  (unnamed)
  0x00416E30  CSWGuiTextParams::SetText
  0x0071EBD0  (unnamed)
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x00410530  CSWGuiManager::AddPanel
  0x00846690  (unnamed)
  0x00734F60  CExoString::CompareNoCase(char*)
  0x00733780  CExoString::~CExoString
  0x00733700  CExoString::Constructor_5
  0x00844860  (unnamed)
  0x0090F610  (unnamed)
  0x0071E910  (unnamed)
  0x0075BB00  (unnamed)
  0x00733540  CExoString::Constructor
  0x0071DEE0  (unnamed)
  0x009103A0  (unnamed)
struct offsets touched (top 15 of [REG + 0xNN], EBP/ESP excluded):
  +0x4CEC [dec 19692]  x14  via EAX,ECX,EDX
  +0x68 [dec 104]  x10  via EAX,ECX,EDX
  +0x310 [dec 784]  x8  via EAX,ECX,EDX
  +0x48 [dec 72]  x4  via EAX,ECX,EDX
  +0x90 [dec 144]  x3  via ECX,EDX
  +0x123 [dec 291]  x3  via EAX,EDX
  +0x4 [dec 4]  x2  via EDX
  +0x1C [dec 28]  x2  via ECX,EDX
  +0xA0 [dec 160]  x2  via ECX
  +0xB6 [dec 182]  x2  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 0090ede0 FUN_0090ede0 size=2082 ret=0 callers=1 callees=20

void __fastcall FUN_0090ede0(int param_1)

{
  int iVar1;
  uint uVar2;
  undefined1 *puStack_130;
  undefined *puStack_12c;
  uint uStack_128;
  undefined4 local_124;
  undefined4 local_120;
  undefined4 local_11c;
  undefined4 local_118;
  undefined4 local_114;
  undefined4 local_110;
  undefined4 local_10c;
  int local_108;
  int local_104;
  int local_100;
  char local_f9;
  ushort local_f8;
  byte local_f5;
  int local_f4;
  int local_f0;
  int local_ec;
  char local_e5;
  ushort local_e4;
  byte local_e1;
  int local_e0;
  int local_dc;
  undefined4 local_d8;
  ushort local_d2;
  int local_d0;
  undefined4 local_cc;
  int local_c8;
  uint local_c4;
  char local_be;
  byte local_bd;
  int local_bc;
  int local_b8;
  uint local_b4;
  undefined4 local_b0;
  int local_ac;
  uint local_a8;
  ushort local_a2;
  int local_a0;
  int local_9c;
  uint local_98;
/* ... 266 more lines truncated by --max-lines */
```

---
## CARD C009
**CLAIM:** Class `CSWGuiSkillsCharGen` has vftable 0x009AA1AC; its constructor is 0x0090C6A0.

```
ASM lines of ctor 0x0090C6A0 containing a 0x9AAxxx constant:
0090c6ea  c70114a79a00          MOV dword ptr [ECX],0x9aa714
0090cac9  6800a79a00            PUSH 0x9aa700
0090cb42  682ca39a00            PUSH 0x9aa32c
0090cb7e  6818a39a00            PUSH 0x9aa318
0090cbbb  68fca29a00            PUSH 0x9aa2fc
0090cbf8  68e0a29a00            PUSH 0x9aa2e0

ANCHORS of ctor:
anchors for 0x0090C6A0 (FUN_0090c6a0)
string literals (46):
  0x009AA700  at 0090cac9  'SKCHRGEN_p'
  0x009A969C  at 0090cae9  'MAIN_TITLE_LBL'
  0x009AA32C  at 0090cb42  'SUB_TITLE_LBL'
  0x009AA318  at 0090cb7e  'REMAINING_BOX_LBL'
  0x009AA2FC  at 0090cbbb  'REMAINING_SELECTIONS_LBL'
  0x009AA2E0  at 0090cbf8  'SELECTIONS_REMAINING_LBL'
  0x009AA6F4  at 0090cc34  'COST_LBL'
  0x009AA6E4  at 0090cc71  'COST_POINTS_LBL'
  0x009AA6D4  at 0090ccae  'CLASSSKL_LBL'
  0x009A1810  at 0090ccea  'LB_DESC'

VTABLE DUMP:
0x9AA1AC (28 slots)
  slot 0 +0x0 = 0x903AA0
  slot 15 +0x3C = 0x903ED0
  slot 18 +0x48 = 0x903BA0
```

---
## CARD C010
**CLAIM:** Hook site at 0x009099A2 inside the function at 0x009098D0: bytes `0f b7 4d e4 51`. At this point choosable-power loop after the 4 vector probes, before the 0x848660 args are pushed: id = word [ebp-0x1C], panel [ebp-0x68]. Consumed-exit (skip) target 0x00909B51 = jmp back to loop head (e9 c8 fd ff ff region); nothing pushed yet.

```
SITE WINDOW (>> = claimed address):
   00909986  0f b7 45 e4        MOVZX EAX,word ptr [EBP + -0x1c]
   0090998A  50                 PUSH EAX
   0090998B  8b 4d 98           MOV ECX,dword ptr [EBP + -0x68]
   0090998E  81 c1 d4 1b 00 00  ADD ECX,0x1bd4
   00909994  e8 47 a9 b0 ff     CALL 0x004142e0
   00909999  83 f8 ff           CMP EAX,-0x1
   0090999C  0f 85 af 01 00 00  JNZ 0x00909b51
>> 009099A2  0f b7 4d e4        MOVZX ECX,word ptr [EBP + -0x1c]
   009099A6  51                 PUSH ECX
   009099A7  8b 4d 98           MOV ECX,dword ptr [EBP + -0x68]
   009099AA  81 c1 ec 1b 00 00  ADD ECX,0x1bec
   009099B0  e8 2b a9 b0 ff     CALL 0x004142e0
   009099B5  83 f8 ff           CMP EAX,-0x1
   009099B8  0f 85 93 01 00 00  JNZ 0x00909b51
   009099BE  8b 55 98           MOV EDX,dword ptr [EBP + -0x68]

EXIT WINDOW:
   00909B43  8b 4d 98           MOV ECX,dword ptr [EBP + -0x68]
   00909B46  81 c1 d4 1b 00 00  ADD ECX,0x1bd4
   00909B4C  e8 0f 4f f3 ff     CALL 0x0083ea60
>> 00909B51  e9 c8 fd ff ff     JMP 0x0090991e
   00909B56  c7 45 fc ff ff ff ff MOV dword ptr [EBP + -0x4],0xffffffff
   00909B5D  8d 4d e8           LEA ECX,[EBP + -0x18]
   00909B60  e8 1b 9c e2 ff     CALL 0x00733780
```

---
## CARD C011
**CLAIM:** Function at 0x0089C170 is `CSWGuiLevelUpIconGrid::SetSlotStateById`: finds the slot holding an id by scanning rows and sets its state; silent no-op when absent.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0089C170, size 170, header ret=8 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x0089C170 (FUN_0089c170)
string literals (0):
direct callees (1):
  0x0089A160  (unnamed)
struct offsets touched (top 1 of [REG + 0xNN], EBP/ESP excluded):
  +0x4 [dec 4]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 0089c170 FUN_0089c170 size=170 ret=8 callers=2 callees=1

void __thiscall FUN_0089c170(int *param_1,int param_2,undefined1 param_3)

{
  uint local_18;
  uint local_c;
  int local_8;
  
  local_8 = 0;
  do {
    if (param_1[1 = byte +0x4] <= local_8) {
      return;
    }
    for (local_18 = 0; (int)local_18 < 3; local_18 = local_18 + 1) {
      if (*(int *)(*(int *)(*param_1 + local_8 * 4) + 0x108 + local_18 * 0xb4) == param_2) {
        local_c = local_18;
        goto LAB_0089c1ed;
      }
    }
    local_c = 0xffffffff;
LAB_0089c1ed:
    if (local_c != 0xffffffff) {
      FUN_0089a160(local_c & 0xff,param_3);
      return;
    }
    local_8 = local_8 + 1;
  } while( true );
}
```

---
## CARD C012
**CLAIM:** Function at 0x0090C6A0 is `CSWGuiFeatsCharGen::Constructor`: feats level-up panel ctor (size 0x1C08, grid ptr at +0x1BF4).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0090C6A0, size 7526, header ret=12 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x0090C6A0 (FUN_0090c6a0)
string literals (46):
  0x009AA700  at 0090cac9  'SKCHRGEN_p'
  0x009A969C  at 0090cae9  'MAIN_TITLE_LBL'
  0x009AA32C  at 0090cb42  'SUB_TITLE_LBL'
  0x009AA318  at 0090cb7e  'REMAINING_BOX_LBL'
  0x009AA2FC  at 0090cbbb  'REMAINING_SELECTIONS_LBL'
  0x009AA2E0  at 0090cbf8  'SELECTIONS_REMAINING_LBL'
  0x009AA6F4  at 0090cc34  'COST_LBL'
  0x009AA6E4  at 0090cc71  'COST_POINTS_LBL'
  0x009AA6D4  at 0090ccae  'CLASSSKL_LBL'
  0x009A1810  at 0090ccea  'LB_DESC'
  0x009A6968  at 0090cd3a  'BTN_ACCEPT'
  0x009AA3C8  at 0090cd80  'BTN_RECOMMENDED'
  0x0099E548  at 0090ce0b  'LBL_BAR1'
  0x0099E53C  at 0090ce51  'LBL_BAR2'
  0x009AA6BC  at 0090cedb  'COMPUTER_USE_POINTS_BTN'
  0x009AA6A8  at 0090cf20  'COMPUTER_USE_LBL'
  0x009AA698  at 0090cf66  'COM_MINUS_BTN'
  0x009AA688  at 0090cfac  'COM_PLUS_BTN'
  0x009AA670  at 0090d059  'DEMOLITIONS_POINTS_BTN'
  0x009AA660  at 0090d09f  'DEMOLITIONS_LBL'
  0x009AA650  at 0090d0e5  'DEM_MINUS_BTN'
  0x009AA640  at 0090d12a  'DEM_PLUS_BTN'
  0x009AA62C  at 0090d1dc  'STEALTH_POINTS_BTN'
  0x009AA620  at 0090d222  'STEALTH_LBL'
  0x009AA610  at 0090d267  'STE_MINUS_BTN'
  0x009AA600  at 0090d2ad  'STE_PLUS_BTN'
  0x009AA5EC  at 0090d35f  'PERSUADE_POINTS_BTN'
  0x009AA5DC  at 0090d3a4  'PERSUADE_LBL'
  0x009AA5CC  at 0090d3ea  'PER_MINUS_BTN'
  0x009AA5BC  at 0090d430  'PER_PLUS_BTN'
  0x009AA5A8  at 0090d4e0  'REPAIR_POINTS_BTN'
  0x009AA59C  at 0090d526  'REPAIR_LBL'

DECOMPILE (offsets annotated; truncated):
// FN 0090c6a0 FUN_0090c6a0 size=7526 ret=12 callers=2 callees=30

undefined4 * __thiscall
FUN_0090c6a0(undefined4 *param_1,undefined4 param_2,undefined4 param_3,uint param_4)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  int local_2cc;
  char local_296;
  char local_28a;
  char local_27a;
  undefined1 local_26a;
  undefined1 local_200 [8];
  undefined1 local_1f8 [8];
  undefined1 local_1f0 [8];
  undefined1 local_1e8 [8];
  undefined1 local_1e0 [8];
  undefined1 local_1d8 [8];
  undefined1 local_1d0 [8];
  undefined1 local_1c8 [8];
  undefined1 local_1c0 [8];
  undefined1 local_1b8 [8];
  undefined1 local_1b0 [8];
  undefined1 local_1a8 [8];
  undefined1 local_1a0 [8];
  undefined1 local_198 [8];
  undefined1 local_190 [8];
  undefined1 local_188 [8];
  undefined1 local_180 [8];
  undefined1 local_178 [8];
  undefined1 local_170 [8];
  undefined1 local_168 [8];
  undefined1 local_160 [8];
  undefined1 local_158 [8];
  undefined1 local_150 [8];
  undefined1 local_148 [8];
  undefined1 local_140 [8];
  undefined1 local_138 [8];
  undefined1 local_130 [8];
  undefined1 local_128 [8];
  undefined1 local_120 [8];
  undefined1 local_118 [8];
  undefined1 local_110 [8];
  undefined1 local_108 [8];
  undefined1 local_100 [8];
  undefined1 local_f8 [8];
  undefined1 local_f0 [8];
/* ... 500 more lines truncated by --max-lines */
```

---
## CARD C013
**CLAIM:** Hook site at 0x0089B6E4 inside the function at 0x0089B600: bytes `89 8d 3c ff ff ff`. At this point loop body of feat grid builder right after lea ecx,[eax+edx*2]: ecx = &featId (ushort) of the current candidate; EAX and flags untouched. Consumed-exit (skip) target 0x0089BE96 = loop increment (mov edx,[ebp-0x28]); nothing pushed yet.

```
SITE WINDOW (>> = claimed address):
   0089B6C6  89 85 40 ff ff ff  MOV dword ptr [EBP + 0xffffff40],EAX
   0089B6CC  8b 4d d8           MOV ECX,dword ptr [EBP + -0x28]
   0089B6CF  3b 8d 40 ff ff ff  CMP ECX,dword ptr [EBP + 0xffffff40]
   0089B6D5  0f 8d c9 07 00 00  JGE 0x0089bea4
   0089B6DB  8b 55 d8           MOV EDX,dword ptr [EBP + -0x28]
   0089B6DE  8b 45 dc           MOV EAX,dword ptr [EBP + -0x24]
   0089B6E1  8d 0c 50           LEA ECX,[EAX + EDX*0x2]
>> 0089B6E4  89 8d 3c ff ff ff  MOV dword ptr [EBP + 0xffffff3c],ECX
   0089B6EA  8b 95 3c ff ff ff  MOV EDX,dword ptr [EBP + 0xffffff3c]
   0089B6F0  0f b7 02           MOVZX EAX,word ptr [EDX]
   0089B6F3  50                 PUSH EAX
   0089B6F4  8b 0d d0 b4 a1 00  MOV ECX,dword ptr [0x00a1b4d0]
   0089B6FA  e8 f1 69 e0 ff     CALL 0x006a20f0
   0089B6FF  89 45 e8           MOV dword ptr [EBP + -0x18],EAX
   0089B702  8b 4d d8           MOV ECX,dword ptr [EBP + -0x28]

EXIT WINDOW:
   0089BE8A  51                 PUSH ECX
   0089BE8B  8b 8d fc fe ff ff  MOV ECX,dword ptr [EBP + 0xfffffefc]
   0089BE91  e8 3a 00 00 00     CALL 0x0089bed0
>> 0089BE96  8b 55 d8           MOV EDX,dword ptr [EBP + -0x28]
   0089BE99  83 c2 01           ADD EDX,0x1
   0089BE9C  89 55 d8           MOV dword ptr [EBP + -0x28],EDX
   0089BE9F  e9 15 f8 ff ff     JMP 0x0089b6b9
```

---
## CARD C014
**CLAIM:** Function at 0x0089C290 is `CSWGuiLevelUpIconGrid::SetAllSlotStates`: sets every slot of every row to the given state (called with 3 = dimmed).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0089C290, size 181, header ret=0 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x0089C290 (FUN_0089c290)
string literals (0):
direct callees (1):
  0x0089A0E0  (unnamed)
struct offsets touched (top 3 of [REG + 0xNN], EBP/ESP excluded):
  +0xD [dec 13]  x4  via EAX,ECX,EDX
  +0xC [dec 12]  x3  via EAX,EDX
  +0x4 [dec 4]  x2  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 0089c290 FUN_0089c290 size=181 ret=0 callers=4 callees=1

undefined4 __fastcall FUN_0089c290(undefined4 *param_1)

{
  undefined4 uVar1;
  
  if ((*(char *)(param_1 + 0x3 [dec 3] {undefined4*: byte +0xc}) != '\0') || (*(char *)((int)param_1 + 0xd) != '\0')) {
    if ((int)(uint)*(byte *)((int)param_1 + 0xd) < (int)param_1[1 = byte +0x4]) {
      FUN_0089a0e0(*(undefined1 *)(param_1 + 0x3 [dec 3] {undefined4*: byte +0xc}),0);
    }
    *(undefined1 *)((int)param_1 + 0xd) = 0;
    *(undefined1 *)(param_1 + 0x3 [dec 3] {undefined4*: byte +0xc}) = 0;
  }
  if ((int)param_1[1 = byte +0x4] < 1) {
    uVar1 = 0xffffffff;
  }
  else {
    FUN_0089a0e0(0,1);
    uVar1 = *(undefined4 *)(*(int *)*param_1 + 0x108);
  }
  return uVar1;
}
```

---
## CARD C015
**CLAIM:** Function at 0x0090F610 is `CSWGuiSkillsCharGen::OnRankUpButton`: skills rank-up button handler: cost via 0x846350, rank change via 0x846690.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0090F610, size 984, header ret=4 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x0090F610 (FUN_0090f610)
string literals (0):
direct callees (11):
  0x00734F60  CExoString::CompareNoCase(char*)
  0x00417180  (unnamed)
  0x00733700  CExoString::Constructor_5
  0x00733780  CExoString::~CExoString
  0x00846350  (unnamed)
  0x009102A0  (unnamed)
  0x00416EA0  (unnamed)
  0x00416E30  CSWGuiTextParams::SetText
  0x00417010  (unnamed)
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x009103A0  (unnamed)
struct offsets touched (top 7 of [REG + 0xNN], EBP/ESP excluded):
  +0x4CEC [dec 19692]  x8  via EAX,ECX,EDX
  +0x38 [dec 56]  x4  via EAX,ECX
  +0x1C [dec 28]  x2  via EAX,ECX
  +0x68 [dec 104]  x2  via EDX
  +0x4CF8 [dec 19704]  x2  via ECX,EDX
  +0x12C [dec 300]  x1  via ECX
  +0x310 [dec 784]  x1  via EAX

DECOMPILE (offsets annotated; truncated):
// FN 0090f610 FUN_0090f610 size=984 ret=4 callers=2 callees=11

void __thiscall FUN_0090f610(int param_1,int param_2)

{
  bool bVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  undefined1 local_24 [8];
  int local_1c;
  int local_18;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00958a08;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  for (local_14 = 0; (int)local_14 < 8; local_14 = local_14 + 1) {
    local_1c = 0;
    if ((*(uint *)(param_1 + 0x4cf8) >> 1 & 1) == 0) {
      local_18 = (local_14 & 0xffff) * 0x20 + *(int *)(DAT_00a1b4d0 + 0x12c [dec 300]);
      bVar1 = false;
      if (local_14 == 2) {
        FUN_0077d800(uVar2);
        iVar3 = FUN_00734f60(&DAT_00995154);
        if (iVar3 != 0) {
          bVar1 = true;
        }
      }
      if ((*(uint *)(local_18 + 0x1c) >> 4 & 1) == 0) {
        local_1c = 1;
      }
      else if ((((*(uint *)(param_1 + 0x4cf8) >> 2 & 1) != 0) &&
               ((*(uint *)(local_18 + 0x1c) >> 5 & 1) == 0)) && (!bVar1)) {
        local_1c = 1;
      }
    }
    if (param_2 == param_1 + 0x1928 + local_14 * 0x1d0) {
      *(uint *)(param_1 + 0x4cec) = local_14;
      if (local_1c == 0) {
        FUN_00417010(&DAT_00a7fdac);
        FUN_00417010(&DAT_00a7fdac);
      }
      else {
/* ... 40 more lines truncated by --max-lines */
```

---
## CARD C016
**CLAIM:** Function at 0x009074E0 is `CSWGuiPowersLevelUp::Constructor`: powers level-up panel ctor (grid ptr at +0x1BF8).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x009074E0, size 2903, header ret=8 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x009074E0 (FUN_009074e0)
string literals (15):
  0x009AA33C  at 009077e9  'pwrlvlup_p'
  0x009A969C  at 00907809  'MAIN_TITLE_LBL'
  0x009AA32C  at 00907843  'SUB_TITLE_LBL'
  0x009AA318  at 00907880  'REMAINING_BOX_LBL'
  0x009AA2FC  at 009078bd  'REMAINING_SELECTIONS_LBL'
  0x009AA2E0  at 009078f9  'SELECTIONS_REMAINING_LBL'
  0x009A1810  at 00907936  'LB_DESC'
  0x009AA2D4  at 00907980  'LB_POWERS'
  0x009AA2C8  at 009079bd  'LBL_POWER'
  0x009AA2B8  at 009079f9  'RECOMMENDED_BTN'
  0x009AA2AC  at 00907a36  'SELECT_BTN'
  0x009AA2A0  at 00907a73  'ACCEPT_BTN'
  0x009AA294  at 00907aaf  'BACK_BTN'
  0x0099E548  at 00907af5  'LBL_BAR1'
  0x0099E53C  at 00907b3b  'LBL_BAR2'
direct callees (25):
  0x0041A510  (unnamed)
  0x0091D67E  (unnamed)
  0x0084A640  (unnamed)
  0x00760EA0  (unnamed)
  0x00733DF0  CExoString::operator+
  0x0041CE90  (unnamed)
  0x00419740  (unnamed)
  0x0040E5A0  CSWGuiPanel::Constructor
  0x007337C0  CExoString::operator=
  0x0073F550  CClientExoApp::GetCreatureByGameObjectID
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x00908540  (unnamed)
  0x0040F5A0  CSWGuiPanel::StopLoadFromLayout
  0x00733780  CExoString::~CExoString
  0x00733570  CExoString::CExoString_char
  0x0040F620  CSWGuiPanel::InitControl

DECOMPILE (offsets annotated; truncated):
// FN 009074e0 FUN_009074e0 size=2903 ret=8 callers=1 callees=25

undefined4 * __thiscall FUN_009074e0(undefined4 *param_1,undefined4 param_2,undefined4 param_3)

{
  uint uVar1;
  undefined4 uVar2;
  int iVar3;
  undefined1 local_dc [8];
  undefined1 local_d4 [8];
  undefined1 local_cc [8];
  undefined1 local_c4 [8];
  undefined1 local_bc [8];
  undefined1 local_b4 [8];
  undefined1 local_ac [8];
  undefined1 local_a4 [8];
  undefined1 local_9c [8];
  undefined1 local_94 [8];
  undefined1 local_8c [8];
  undefined1 local_84 [8];
  undefined1 local_7c [8];
  undefined1 local_74 [8];
  undefined1 local_6c [8];
  undefined1 local_64 [8];
  undefined1 local_5c [8];
  undefined1 local_54 [8];
  undefined1 local_4c [8];
  undefined1 local_44 [8];
  undefined1 local_3c [8];
  undefined1 local_34 [8];
  undefined1 local_2c [16];
  int local_1c;
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009808b5;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_0040e5a0(param_2);
  local_8 = 0;
  *param_1 = CSWGuiPowersLevelUp::vftable;
  FUN_00419740(uVar1);
  local_8._0_1_ = 1;
  FUN_00419740();
  local_8._0_1_ = 2;
/* ... 186 more lines truncated by --max-lines */
```

---
## CARD C017
**CLAIM:** Function at 0x00909360 is `CSWGuiPowersLevelUp::RefreshGridStates`: repaint power grid slot states (spells 6 and 0xE forced to 3 when panel+0x1BAC bit0 == 0).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x00909360, size 644, header ret=0 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x00909360 (FUN_00909360)
string literals (0):
direct callees (10):
  0x00733700  CExoString::Constructor_5
  0x00733780  CExoString::~CExoString
  0x0041FC30  (unnamed)
  0x0089C170  (unnamed)
  0x0089C070  (unnamed)
  0x0089C220  (unnamed)
  0x0089C290  (unnamed)
  0x00416E30  CSWGuiTextParams::SetText
  0x0041E870  (unnamed)
  0x00908F70  (unnamed)
struct offsets touched (top 12 of [REG + 0xNN], EBP/ESP excluded):
  +0x1BB0 [dec 7088]  x4  via ECX,EDX
  +0x1BD4 [dec 7124]  x4  via ECX,EDX
  +0x1BA8 [dec 7080]  x1  via EAX
  +0x1BAC [dec 7084]  x1  via EDX
  +0x1BC8 [dec 7112]  x1  via ECX
  +0x1BCC [dec 7116]  x1  via ECX
  +0x1BD8 [dec 7128]  x1  via EDX
  +0x1BE0 [dec 7136]  x1  via EAX
  +0x1BE4 [dec 7140]  x1  via EAX
  +0x1BEC [dec 7148]  x1  via ECX
  +0x1BF0 [dec 7152]  x1  via ECX
  +0x1C05 [dec 7173]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 00909360 FUN_00909360 size=644 ret=0 callers=3 callees=10

void __fastcall FUN_00909360(int param_1)

{
  undefined4 uVar1;
  undefined1 local_1c [8];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a238;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0089c220(3);
  for (local_14 = 0; local_14 < *(int *)(param_1 + 0x1bcc); local_14 = local_14 + 1) {
    FUN_0089c170(*(undefined4 *)(*(int *)(param_1 + 0x1bc8) + local_14 * 4),1);
  }
  for (local_14 = 0; local_14 < *(int *)(param_1 + 0x1be4); local_14 = local_14 + 1) {
    FUN_0089c170(*(undefined4 *)(*(int *)(param_1 + 0x1be0) + local_14 * 4),4);
  }
  for (local_14 = 0; local_14 < *(int *)(param_1 + 0x1bd8); local_14 = local_14 + 1) {
    if (((*(uint *)(param_1 + 0x1bac) & 1) == 0) &&
       ((*(int *)(*(int *)(param_1 + 0x1bd4) + local_14 * 4) == 6 ||
        (*(int *)(*(int *)(param_1 + 0x1bd4) + local_14 * 4) == 0xe)))) {
      FUN_0089c170(*(undefined4 *)(*(int *)(param_1 + 0x1bd4) + local_14 * 4),3);
    }
    else {
      FUN_0089c170(*(undefined4 *)(*(int *)(param_1 + 0x1bd4) + local_14 * 4),0);
    }
  }
  for (local_14 = 0; local_14 < *(int *)(param_1 + 0x1bf0); local_14 = local_14 + 1) {
    FUN_0089c170(*(undefined4 *)(*(int *)(param_1 + 0x1bec) + local_14 * 4),2);
  }
  if (*(int *)(param_1 + 0x1bb0) == -1) {
    FUN_0041fc30(param_1 + 0x1bf8,0,1,0);
    uVar1 = FUN_0089c290();
    *(undefined4 *)(param_1 + 0x1bb0) = uVar1;
  }
  FUN_0089c070(*(undefined4 *)(param_1 + 0x1bb0));
  FUN_0041e870(*(undefined1 *)(param_1 + 0x1c05),0);
  FUN_00908f70(*(undefined4 *)(param_1 + 0x1bb0));
  FUN_00733700(*(undefined1 *)(param_1 + 0x1ba8));
  local_8 = 0;
  FUN_00416e30(local_1c);
  local_8 = 0xffffffff;
  FUN_00733780();
/* ... 3 more lines truncated by --max-lines */
```

---
## CARD C018
**CLAIM:** Function at 0x0089C070 is `CSWGuiLevelUpIconGrid::SetSlotStateById`: finds the slot holding an id by scanning rows and sets its state; silent no-op when absent.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0089C070, size 250, header ret=4 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x0089C070 (FUN_0089c070)
string literals (0):
direct callees (1):
  0x0089A0E0  (unnamed)
struct offsets touched (top 3 of [REG + 0xNN], EBP/ESP excluded):
  +0xC [dec 12]  x4  via EAX,ECX,EDX
  +0xD [dec 13]  x4  via EAX,ECX,EDX
  +0x4 [dec 4]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 0089c070 FUN_0089c070 size=250 ret=4 callers=5 callees=1

void __thiscall FUN_0089c070(int *param_1,int param_2)

{
  uint local_18;
  uint local_c;
  uint local_8;
  
  local_8 = 0;
  do {
    if (param_1[1 = byte +0x4] <= (int)local_8) {
      return;
    }
    for (local_18 = 0; (int)local_18 < 3; local_18 = local_18 + 1) {
      if (*(int *)(*(int *)(*param_1 + local_8 * 4) + 0x108 + local_18 * 0xb4) == param_2) {
        local_c = local_18;
        goto LAB_0089c0f1;
      }
    }
    local_c = 0xffffffff;
LAB_0089c0f1:
    if (local_c != 0xffffffff) {
      if ((*(byte *)(param_1 + 0x3 [dec 3] {int*: byte +0xc}) != local_c) || (*(byte *)((int)param_1 + 0xd) != local_8)) {
        FUN_0089a0e0((char)param_1[3 = byte +0xc],0);
        *(undefined1 *)(param_1 + 0x3 [dec 3] {int*: byte +0xc}) = (undefined1)local_c;
        *(undefined1 *)((int)param_1 + 0xd) = (undefined1)local_8;
      }
      FUN_0089a0e0((char)param_1[3 = byte +0xc],1);
      return;
    }
    local_8 = local_8 + 1;
  } while( true );
}
```

---
## CARD C019
**CLAIM:** Class `CSWGuiFeatsCharGen` has vftable 0x009AA34C; its constructor is 0x00909E00.

```
ASM lines of ctor 0x00909E00 containing a 0x9AAxxx constant:
00909e4a  c70134a49a00          MOV dword ptr [ECX],0x9aa434
0090a0d2  6824a49a00            PUSH 0x9aa424
0090a12c  682ca39a00            PUSH 0x9aa32c
0090a169  6818a39a00            PUSH 0x9aa318
0090a1a5  6804a49a00            PUSH 0x9aa404
0090a1e2  68e4a39a00            PUSH 0x9aa3e4

ANCHORS of ctor:
anchors for 0x00909E00 (FUN_00909e00)
string literals (14):
  0x009AA424  at 0090a0d2  'FTCHRGEN_p'
  0x009A969C  at 0090a0f2  'MAIN_TITLE_LBL'
  0x009AA32C  at 0090a12c  'SUB_TITLE_LBL'
  0x009AA318  at 0090a169  'REMAINING_BOX_LBL'
  0x009AA404  at 0090a1a5  'STD_REMAINING_SELECTIONS_LBL'
  0x009AA3E4  at 0090a1e2  'STD_SELECTIONS_REMAINING_LBL'
  0x009A1810  at 0090a21f  'LB_DESC'
  0x009AA3D8  at 0090a268  'LB_FEATS'
  0x0099D72C  at 0090a2a4  'LBL_NAME'
  0x009A6968  at 0090a2e1  'BTN_ACCEPT'

VTABLE DUMP:
0x9AA34C (28 slots)
  slot 0 +0x0 = 0x908040
  slot 15 +0x3C = 0x908920
  slot 18 +0x48 = 0x908460
```

---
## CARD C020
**CLAIM:** Function at 0x0089C290 is `CSWGuiLevelUpIconGrid::HandleNavKey`: arrow-key navigation over rows/columns, skipping empty (-1) slots.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0089C290, size 181, header ret=0 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x0089C290 (FUN_0089c290)
string literals (0):
direct callees (1):
  0x0089A0E0  (unnamed)
struct offsets touched (top 3 of [REG + 0xNN], EBP/ESP excluded):
  +0xD [dec 13]  x4  via EAX,ECX,EDX
  +0xC [dec 12]  x3  via EAX,EDX
  +0x4 [dec 4]  x2  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 0089c290 FUN_0089c290 size=181 ret=0 callers=4 callees=1

undefined4 __fastcall FUN_0089c290(undefined4 *param_1)

{
  undefined4 uVar1;
  
  if ((*(char *)(param_1 + 0x3 [dec 3] {undefined4*: byte +0xc}) != '\0') || (*(char *)((int)param_1 + 0xd) != '\0')) {
    if ((int)(uint)*(byte *)((int)param_1 + 0xd) < (int)param_1[1 = byte +0x4]) {
      FUN_0089a0e0(*(undefined1 *)(param_1 + 0x3 [dec 3] {undefined4*: byte +0xc}),0);
    }
    *(undefined1 *)((int)param_1 + 0xd) = 0;
    *(undefined1 *)(param_1 + 0x3 [dec 3] {undefined4*: byte +0xc}) = 0;
  }
  if ((int)param_1[1 = byte +0x4] < 1) {
    uVar1 = 0xffffffff;
  }
  else {
    FUN_0089a0e0(0,1);
    uVar1 = *(undefined4 *)(*(int *)*param_1 + 0x108);
  }
  return uVar1;
}
```

---
## CARD C021
**CLAIM:** Hook site at 0x0089AB29 inside the function at 0x0089AAA0: bytes `89 4d b8 8b 15 d0 b4 a1 00`. At this point power grid builder loop after mov ecx,[eax]: ecx = spellId (uint); window EAX-free. Consumed-exit (skip) target 0x0089AEB3 = loop increment (mov eax,[ebp-0x10]).

```
SITE WINDOW (>> = claimed address):
   0089AB12  0f 8d ae 03 00 00  JGE 0x0089aec6
   0089AB18  8b 45 f0           MOV EAX,dword ptr [EBP + -0x10]
   0089AB1B  8b 4d d8           MOV ECX,dword ptr [EBP + -0x28]
   0089AB1E  8d 14 81           LEA EDX,[ECX + EAX*0x4]
   0089AB21  89 55 c0           MOV dword ptr [EBP + -0x40],EDX
   0089AB24  8b 45 c0           MOV EAX,dword ptr [EBP + -0x40]
   0089AB27  8b 08              MOV ECX,dword ptr [EAX]
>> 0089AB29  89 4d b8           MOV dword ptr [EBP + -0x48],ECX
   0089AB2C  8b 15 d0 b4 a1 00  MOV EDX,dword ptr [0x00a1b4d0]
   0089AB32  89 55 bc           MOV dword ptr [EBP + -0x44],EDX
   0089AB35  8b 45 b8           MOV EAX,dword ptr [EBP + -0x48]
   0089AB38  50                 PUSH EAX
   0089AB39  8b 4d bc           MOV ECX,dword ptr [EBP + -0x44]
   0089AB3C  8b 89 04 01 00 00  MOV ECX,dword ptr [ECX + 0x104]
   0089AB42  e8 09 69 e2 ff     CALL 0x006c1450

EXIT WINDOW:
   0089AEAC  8b 14 81           MOV EDX,dword ptr [ECX + EAX*0x4]
   0089AEAF  52                 PUSH EDX
   0089AEB0  8b 4d 84           MOV ECX,dword ptr [EBP + -0x7c]
>> 0089AEB3  e8 78 00 00 00     CALL 0x0089af30
   0089AEB8  8b 45 f0           MOV EAX,dword ptr [EBP + -0x10]
   0089AEBB  83 c0 01           ADD EAX,0x1
   0089AEBE  89 45 f0           MOV dword ptr [EBP + -0x10],EAX
```

---
## CARD C022
**CLAIM:** Hook site at 0x0090C2BE inside the function at 0x0090C230: bytes `0f b7 55 e4 52`. At this point choosable-feat loop before the 4 vector probes: id = word [ebp-0x1C], panel [ebp-0x28], S [ebp-0x10]. Consumed-exit (skip) target 0x0090C365 = jmp back to loop head 0x90C27F; nothing pushed yet.

```
SITE WINDOW (>> = claimed address):
   0090C29D  39 45 e0           CMP dword ptr [EBP + -0x20],EAX
   0090C2A0  0f 8d c4 00 00 00  JGE 0x0090c36a
   0090C2A6  0f b7 4d e0        MOVZX ECX,word ptr [EBP + -0x20]
   0090C2AA  8b 15 d0 b4 a1 00  MOV EDX,dword ptr [0x00a1b4d0]
   0090C2B0  8b 82 18 01 00 00  MOV EAX,dword ptr [EDX + 0x118]
   0090C2B6  66 8b 0c 48        MOV CX,word ptr [EAX + ECX*0x2]
   0090C2BA  66 89 4d e4        MOV word ptr [EBP + -0x1c],CX
>> 0090C2BE  0f b7 55 e4        MOVZX EDX,word ptr [EBP + -0x1c]
   0090C2C2  52                 PUSH EDX
   0090C2C3  8b 4d d8           MOV ECX,dword ptr [EBP + -0x28]
   0090C2C6  81 c1 a8 1b 00 00  ADD ECX,0x1ba8
   0090C2CC  e8 4f 03 00 00     CALL 0x0090c620
   0090C2D1  83 f8 ff           CMP EAX,-0x1
   0090C2D4  0f 85 8b 00 00 00  JNZ 0x0090c365
   0090C2DA  0f b7 45 e4        MOVZX EAX,word ptr [EBP + -0x1c]

EXIT WINDOW:
   0090C357  8b 4d d8           MOV ECX,dword ptr [EBP + -0x28]
   0090C35A  81 c1 c0 1b 00 00  ADD ECX,0x1bc0
   0090C360  e8 eb 70 c8 ff     CALL 0x00593450
>> 0090C365  e9 15 ff ff ff     JMP 0x0090c27f
   0090C36A  c7 45 fc ff ff ff ff MOV dword ptr [EBP + -0x4],0xffffffff
   0090C371  8d 4d e8           LEA ECX,[EBP + -0x18]
   0090C374  e8 07 74 e2 ff     CALL 0x00733780
```


# BATCH 2 (cards C023-C044)

---
## CARD C023
**CLAIM:** Function at 0x0089BED0 is `CSWGuiLevelUpIconGrid::AddFeatRow`: allocs 0x324 grid row for (baseFeat, tier2, tier3); ids at row+0x108 + c*0xB4.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0089BED0, size 403, header ret=12 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x0089BED0 (FUN_0089bed0)
string literals (0):
direct callees (11):
  0x00710EB0  (unnamed)
  0x007107F0  (unnamed)
  0x009196EE  __security_check_cookie
  0x00416090  (unnamed)
  0x0091C860  _ftol2_sse
  0x00899930  (unnamed)
  0x0083EA60  CExoArrayList<uint32>::Add
  0x00919723  operator new
  0x00710CA0  (unnamed)
  0x00479AB0  (unnamed)
  0x006A20F0  (unnamed)
struct offsets touched (top 1 of [REG + 0xNN], EBP/ESP excluded):
  +0x4 [dec 4]  x1  via EAX

DECOMPILE (offsets annotated; truncated):
// FN 0089bed0 FUN_0089bed0 size=403 ret=12 callers=2 callees=11

void FUN_0089bed0(int param_1,undefined4 param_2,undefined4 param_3)

{
  uint uVar1;
  int *piVar2;
  float10 fVar3;
  undefined4 extraout_var;
  undefined4 uVar4;
  undefined4 local_78;
  undefined4 local_74;
  undefined4 local_70;
  undefined4 local_6c;
  undefined1 local_68 [16];
  undefined1 local_58 [16];
  int local_48;
  int *local_44;
  undefined1 local_40 [16];
  uint local_30;
  float local_2c;
  int *local_28;
  int local_24[5 = byte +0x14];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00976c7b;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_30 = uVar1;
  FUN_007107f0(uVar1);
  uVar4 = extraout_var;
  local_48 = FUN_00919723(0x324);
  local_8 = 0;
  if (local_48 == 0) {
    piVar2 = (int *)0x0;
  }
  else {
    piVar2 = (int *)FUN_00899930();
  }
  local_8 = 0xffffffff;
  local_24[0] = param_1;
  local_24[1 = byte +0x4] = param_2;
  local_24[2 = byte +0x8] = param_3;
  local_44 = piVar2;
  local_28 = piVar2;
/* ... 23 more lines truncated by --max-lines */
```

---
## CARD C024
**CLAIM:** Class `CSWGuiPowersLevelUp` has vftable 0x009AA34C; its constructor is 0x009074E0.

```
ASM lines of ctor 0x009074E0 containing a 0x9AAxxx constant:
0090752a  c7014ca39a00          MOV dword ptr [ECX],0x9aa34c
009077e9  683ca39a00            PUSH 0x9aa33c
00907843  682ca39a00            PUSH 0x9aa32c
00907880  6818a39a00            PUSH 0x9aa318
009078bd  68fca29a00            PUSH 0x9aa2fc
009078f9  68e0a29a00            PUSH 0x9aa2e0

ANCHORS of ctor:
anchors for 0x009074E0 (FUN_009074e0)
string literals (15):
  0x009AA33C  at 009077e9  'pwrlvlup_p'
  0x009A969C  at 00907809  'MAIN_TITLE_LBL'
  0x009AA32C  at 00907843  'SUB_TITLE_LBL'
  0x009AA318  at 00907880  'REMAINING_BOX_LBL'
  0x009AA2FC  at 009078bd  'REMAINING_SELECTIONS_LBL'
  0x009AA2E0  at 009078f9  'SELECTIONS_REMAINING_LBL'
  0x009A1810  at 00907936  'LB_DESC'
  0x009AA2D4  at 00907980  'LB_POWERS'
  0x009AA2C8  at 009079bd  'LBL_POWER'
  0x009AA2B8  at 009079f9  'RECOMMENDED_BTN'

VTABLE DUMP:
0x9AA34C (28 slots)
  slot 0 +0x0 = 0x908040
  slot 15 +0x3C = 0x908920
  slot 18 +0x48 = 0x908460
```

---
## CARD C025
**CLAIM:** Function at 0x0090C230 is `CSWGuiPowersLevelUp::BuildChoosablePowerVector`: builds panel+0x1BD4 choosable power vector, 0x848660 per spell id.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0090C230, size 344, header ret=0 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x0090C230 (FUN_0090c230)
string literals (0):
direct callees (5):
  0x00733780  CExoString::~CExoString
  0x00593450  CExoArrayList<uint16>::Add
  0x00733540  CExoString::Constructor
  0x008480B0  CSWCLevelUpStats::CanChooseFeat
  0x0090C620  (unnamed)
struct offsets touched (top 6 of [REG + 0xNN], EBP/ESP excluded):
  +0x68 [dec 104]  x1  via EAX
  +0x118 [dec 280]  x1  via EDX
  +0x11E [dec 286]  x1  via ECX
  +0x310 [dec 784]  x1  via ECX
  +0x1BDC [dec 7132]  x1  via EAX
  +0x1BDD [dec 7133]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 0090c230 FUN_0090c230 size=344 ret=0 callers=3 callees=5

void __fastcall FUN_0090c230(int param_1)

{
  undefined2 uVar1;
  int iVar2;
  uint local_24;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00968348;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  for (local_24 = 0; (int)local_24 < (int)(uint)*(ushort *)(DAT_00a1b4d0 + 0x11e);
      local_24 = local_24 + 1) {
    uVar1 = *(undefined2 *)(*(int *)(DAT_00a1b4d0 + 0x118) + (local_24 & 0xffff) * 2);
    iVar2 = FUN_0090c620(uVar1);
    if (iVar2 == -1) {
      iVar2 = FUN_0090c620(uVar1);
      if (iVar2 == -1) {
        iVar2 = FUN_0090c620(uVar1);
        if (iVar2 == -1) {
          iVar2 = FUN_0090c620(uVar1);
          if (iVar2 == -1) {
            iVar2 = FUN_008480b0(uVar1,*(undefined1 *)(param_1 + 0x1bdc),
                                 *(undefined1 *)(param_1 + 0x1bdd),param_1 + 0x1bcc);
            if (iVar2 != 0) {
              FUN_00593450(uVar1);
            }
          }
        }
      }
    }
  }
  local_8 = 0xffffffff;
  FUN_00733780();
  ExceptionList = local_10;
  return;
}
```

---
## CARD C026
**CLAIM:** Call site at 0x0090C349 in the function at 0x0090C230: bytes `e8 62 bd f3 ff` = call to 0x00848660 [unnamed-in-DB power-availability test (candidate in this packet)]; CanChooseFeat(S, feat, +0x1BDC, +0x1BDD, &selected) with ecx=S=[ebp-0x10], 4 stack args, callee-cleaned RET 0x10; true result pushes into +0x1BC0.

```
CALL WINDOW (>> = claimed address):
   0090C32E  0f b6 91 dd 1b 00 00 MOVZX EDX,byte ptr [ECX + 0x1bdd]
   0090C335  52                 PUSH EDX
   0090C336  8b 45 d8           MOV EAX,dword ptr [EBP + -0x28]
   0090C339  0f b6 88 dc 1b 00 00 MOVZX ECX,byte ptr [EAX + 0x1bdc]
   0090C340  51                 PUSH ECX
   0090C341  0f b7 55 e4        MOVZX EDX,word ptr [EBP + -0x1c]
   0090C345  52                 PUSH EDX
   0090C346  8b 4d f0           MOV ECX,dword ptr [EBP + -0x10]
>> 0090C349  e8 62 bd f3 ff     CALL 0x008480b0
   0090C34E  85 c0              TEST EAX,EAX
   0090C350  74 13              JZ 0x0090c365
   0090C352  0f b7 45 e4        MOVZX EAX,word ptr [EBP + -0x1c]
   0090C356  50                 PUSH EAX
```

---
## CARD C027
**CLAIM:** Call site at 0x0090C349 in the function at 0x0090C230: bytes `e8 62 bd f3 ff` = call to 0x008480B0 [CanChooseFeat (existing DB, high)]; CanChooseFeat(S, feat, +0x1BDC, +0x1BDD, &selected) with ecx=S=[ebp-0x10], 4 stack args, callee-cleaned RET 0x10; true result pushes into +0x1BC0.

```
CALL WINDOW (>> = claimed address):
   0090C32E  0f b6 91 dd 1b 00 00 MOVZX EDX,byte ptr [ECX + 0x1bdd]
   0090C335  52                 PUSH EDX
   0090C336  8b 45 d8           MOV EAX,dword ptr [EBP + -0x28]
   0090C339  0f b6 88 dc 1b 00 00 MOVZX ECX,byte ptr [EAX + 0x1bdc]
   0090C340  51                 PUSH ECX
   0090C341  0f b7 55 e4        MOVZX EDX,word ptr [EBP + -0x1c]
   0090C345  52                 PUSH EDX
   0090C346  8b 4d f0           MOV ECX,dword ptr [EBP + -0x10]
>> 0090C349  e8 62 bd f3 ff     CALL 0x008480b0
   0090C34E  85 c0              TEST EAX,EAX
   0090C350  74 13              JZ 0x0090c365
   0090C352  0f b7 45 e4        MOVZX EAX,word ptr [EBP + -0x1c]
   0090C356  50                 PUSH EAX
```

---
## CARD C028
**CLAIM:** Function at 0x0089A0E0 is `CSWGuiLevelUpIconGrid::SetSlotState`: sets grid slot state byte/colour: 0 normal 1/2 known-granted 3 dimmed 4 selected 5/6 misc.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0089A0E0, size 127, header ret=8 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x0089A0E0 (FUN_0089a0e0)
string literals (0):
direct callees (1):
  0x0089A160  (unnamed)
struct offsets touched (top 0 of [REG + 0xNN], EBP/ESP excluded):

DECOMPILE (offsets annotated; truncated):
// FN 0089a0e0 FUN_0089a0e0 size=127 ret=8 callers=3 callees=1

void __thiscall FUN_0089a0e0(int param_1,byte param_2,uint param_3)

{
  if ((*(uint *)(param_1 + 0x110 + (uint)param_2 * 0xb4) & 1) != param_3) {
    *(uint *)(param_1 + 0x110 + (uint)param_2 * 0xb4) =
         *(uint *)(param_1 + 0x110 + (uint)param_2 * 0xb4) & 0xfffffffe | param_3 & 1;
    FUN_0089a160(param_2,*(undefined1 *)(param_1 + 0x10c + (uint)param_2 * 0xb4));
  }
  return;
}
```

---
## CARD C029
**CLAIM:** Function at 0x0090B360 is `CSWGuiFeatsCharGen::OnRecommendedButton`: feats Recommended button: AddDefaultPackageFeats path (ignores grid filter).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0090B360, size 203, header ret=0 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x0090B360 (FUN_0090b360)
string literals (0):
direct callees (5):
  0x009150E0  (unnamed)
  0x004109C0  CSWGuiManager::PopModalPanel
  0x00904780  (unnamed)
  0x0040FE60  CSWGuiPanel::SetVisible
  0x00844560  (unnamed)
struct offsets touched (top 6 of [REG + 0xNN], EBP/ESP excluded):
  +0x48 [dec 72]  x4  via EAX,ECX
  +0x6C [dec 108]  x4  via EAX,ECX,EDX
  +0x68 [dec 104]  x3  via EAX,ECX
  +0x310 [dec 784]  x2  via ECX,EDX
  +0x1BE0 [dec 7136]  x2  via ECX
  +0x1C [dec 28]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 0090b360 FUN_0090b360 size=203 ret=0 callers=1 callees=5

void __fastcall FUN_0090b360(int param_1)

{
  if (((*(int *)(param_1 + 0x68) != 0) && (*(int *)(*(int *)(param_1 + 0x68) + 0x310) != 0)) &&
     ((*(uint *)(param_1 + 0x1be0) & 1) == 0)) {
    FUN_00844560(param_1,*(undefined4 *)(*(int *)(param_1 + 0x68) + 0x310));
  }
  if (*(int *)(param_1 + 0x6c) != 0) {
    FUN_004109c0();
    FUN_0040fe60(1);
    *(uint *)(param_1 + 0x48) = *(uint *)(param_1 + 0x48) & 0xffffff7f;
    *(uint *)(param_1 + 0x48) = *(uint *)(param_1 + 0x48) & 0xfffffcff | 0x200;
    if ((*(uint *)(param_1 + 0x1be0) & 1) == 0) {
      FUN_009150e0();
    }
    else {
      FUN_00904780();
    }
  }
  return;
}
```

---
## CARD C030
**CLAIM:** Function at 0x00846350 is `CSWCLevelUpStats::GetSkillPointCost`: returns 1 for class skill, 2 cross-class (2 - classSkill).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x00846350, size 276, header ret=8 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x00846350 (FUN_00846350)
string literals (0):
direct callees (4):
  0x00844E40  CSWSCreatureStats::GetClassByPosition
  0x006F24D0  (unnamed)
  0x00844FF0  (unnamed)
  0x006F2440  (unnamed)
struct offsets touched (top 4 of [REG + 0xNN], EBP/ESP excluded):
  +0x1C [dec 28]  x1  via EDX
  +0x32 [dec 50]  x1  via EDX
  +0x124 [dec 292]  x1  via ECX
  +0x12C [dec 300]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 00846350 FUN_00846350 size=276 ret=8 callers=3 callees=4

int __thiscall FUN_00846350(int param_1,ushort param_2,int param_3)

{
  byte bVar1;
  byte bVar2;
  int iVar3;
  bool bVar4;
  uint local_10;
  int local_c;
  
  bVar1 = 0;
  local_c = 0;
  iVar3 = (uint)param_2 * 0x20 + *(int *)(DAT_00a1b4d0 + 0x12c [dec 300]);
  if (iVar3 == 0) {
    local_c = 0;
  }
  else {
    bVar4 = (*(uint *)(iVar3 + 0x1c) >> 3 & 1) != 0;
    for (local_10 = 0; (int)local_10 < (int)(uint)*(byte *)(param_1 + 0x32); local_10 = local_10 + 1
        ) {
      bVar2 = FUN_00844e40(local_10 & 0xff);
      if (((uint)bVar2 * 0x238 + *(int *)(DAT_00a1b4d0 + 0x124) != 0) &&
         (iVar3 = FUN_006f24d0(param_2), iVar3 != 0)) {
        bVar4 = true;
        iVar3 = FUN_006f2440(param_2);
        if ((iVar3 != 0) || (iVar3 = FUN_00844ff0(param_2), iVar3 != 0)) {
          bVar1 = 1;
          break;
        }
      }
    }
    if ((bVar4) && (local_c = 2 - (uint)bVar1, param_3 == 0)) {
      local_c = -local_c;
    }
  }
  return local_c;
}
```

---
## CARD C031
**CLAIM:** Function at 0x0090BF90 is `CSWGuiFeatsCharGen::RefreshGridStates`: repaint feat grid slot states: 0x89C220(3) then known->1 selected->4 choosable->0 granted->2.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0090BF90, size 395, header ret=4 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x0090BF90 (FUN_0090bf90)
string literals (0):
direct callees (9):
  0x00733C40  (unnamed)
  0x00733780  CExoString::~CExoString
  0x005934C0  (unnamed)
  0x00593450  CExoArrayList<uint16>::Add
  0x005AA2C0  (unnamed)
  0x0090C230  (unnamed)
  0x006D6FE0  (unnamed)
  0x0090C620  (unnamed)
  0x006A20F0  (unnamed)
struct offsets touched (top 3 of [REG + 0xNN], EBP/ESP excluded):
  +0x1BDE [dec 7134]  x2  via ECX,EDX
  +0x1BCC [dec 7116]  x1  via ECX
  +0x1BD0 [dec 7120]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 0090bf90 FUN_0090bf90 size=395 ret=4 callers=2 callees=9

void __thiscall FUN_0090bf90(int param_1,undefined2 param_2)

{
  uint uVar1;
  undefined4 uVar2;
  undefined1 *puVar3;
  undefined1 local_2c [8];
  int local_24;
  int local_20;
  int local_1c;
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00981180;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_20 = FUN_0090c620(param_2);
  if (local_20 != -1) {
    FUN_005aa2c0(local_20);
    local_1c = 0;
    puVar3 = local_18;
    FUN_006a20f0(param_2);
    FUN_006d6fe0(puVar3);
    local_8 = 0;
    local_20 = 0;
    while ((local_1c == 0 && (local_20 < *(int *)(param_1 + 0x1bd0)))) {
      puVar3 = local_2c;
      FUN_006a20f0(*(undefined2 *)(*(int *)(param_1 + 0x1bcc) + local_20 * 2));
      uVar2 = FUN_006d6fe0(puVar3);
      local_8._0_1_ = 1;
      local_24 = FUN_00733c40(local_18);
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_00733780(uVar1,uVar2);
      if (local_24 == 0) {
        local_20 = local_20 + 1;
      }
      else {
        local_1c = 1;
      }
    }
    *(char *)(param_1 + 0x1bde) = *(char *)(param_1 + 0x1bde) + -1;
    if (local_1c == 0) {
      FUN_00593450(param_2);
/* ... 11 more lines truncated by --max-lines */
```

---
## CARD C032
**CLAIM:** Function at 0x00846350 is `CSWCLevelUpStats::DecrementSkillRank`: skill rank-down; refunds 1 (class skill) or 2 (cross-class) points.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x00846350, size 276, header ret=8 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x00846350 (FUN_00846350)
string literals (0):
direct callees (4):
  0x00844E40  CSWSCreatureStats::GetClassByPosition
  0x006F24D0  (unnamed)
  0x00844FF0  (unnamed)
  0x006F2440  (unnamed)
struct offsets touched (top 4 of [REG + 0xNN], EBP/ESP excluded):
  +0x1C [dec 28]  x1  via EDX
  +0x32 [dec 50]  x1  via EDX
  +0x124 [dec 292]  x1  via ECX
  +0x12C [dec 300]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 00846350 FUN_00846350 size=276 ret=8 callers=3 callees=4

int __thiscall FUN_00846350(int param_1,ushort param_2,int param_3)

{
  byte bVar1;
  byte bVar2;
  int iVar3;
  bool bVar4;
  uint local_10;
  int local_c;
  
  bVar1 = 0;
  local_c = 0;
  iVar3 = (uint)param_2 * 0x20 + *(int *)(DAT_00a1b4d0 + 0x12c [dec 300]);
  if (iVar3 == 0) {
    local_c = 0;
  }
  else {
    bVar4 = (*(uint *)(iVar3 + 0x1c) >> 3 & 1) != 0;
    for (local_10 = 0; (int)local_10 < (int)(uint)*(byte *)(param_1 + 0x32); local_10 = local_10 + 1
        ) {
      bVar2 = FUN_00844e40(local_10 & 0xff);
      if (((uint)bVar2 * 0x238 + *(int *)(DAT_00a1b4d0 + 0x124) != 0) &&
         (iVar3 = FUN_006f24d0(param_2), iVar3 != 0)) {
        bVar4 = true;
        iVar3 = FUN_006f2440(param_2);
        if ((iVar3 != 0) || (iVar3 = FUN_00844ff0(param_2), iVar3 != 0)) {
          bVar1 = 1;
          break;
        }
      }
    }
    if ((bVar4) && (local_c = 2 - (uint)bVar1, param_3 == 0)) {
      local_c = -local_c;
    }
  }
  return local_c;
}
```

---
## CARD C033
**CLAIM:** Function at 0x00848660 is `CSWCLevelUpStats::CanChooseFeat`: feat availability test: not 0xCF/0xEE, not known, not selected, prereq 0x846B20, slot accounting.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x00848660, size 326, header ret=12 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x00848660 (FUN_00848660)
string literals (0):
direct callees (5):
  0x006C1450  (unnamed)
  0x0084A170  (unnamed)
  0x00844E40  CSWSCreatureStats::GetClassByPosition
  0x006C10A0  (unnamed)
  0x00846DD0  (unnamed)
struct offsets touched (top 4 of [REG + 0xNN], EBP/ESP excluded):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x14C [dec 332]  x2  via EAX
  +0x32 [dec 50]  x1  via EAX
  +0x104 [dec 260]  x1  via EDX

DECOMPILE (offsets annotated; truncated):
// FN 00848660 FUN_00848660 size=326 ret=12 callers=2 callees=5

undefined4 __thiscall FUN_00848660(int param_1,int param_2,byte param_3,int *param_4)

{
  undefined1 uVar1;
  char cVar2;
  int iVar3;
  undefined4 uVar4;
  int iVar5;
  ushort local_10;
  
  cVar2 = *(char *)(param_1 + 0x32);
  iVar3 = FUN_006c1450(param_2);
  if (iVar3 == 0) {
    uVar4 = 0;
  }
  else if ((*(char *)(iVar3 + 0x14c) == '\x01') || (*(char *)(iVar3 + 0x14c) == '\x05')) {
    uVar1 = FUN_00844e40(cVar2 + -1);
    cVar2 = FUN_006c10a0(uVar1);
    if (cVar2 == -1) {
      uVar4 = 0;
    }
    else if (param_3 == 0) {
      uVar4 = 0;
    }
    else {
      iVar5 = FUN_0084a170(0,param_2);
      if (iVar5 == 0) {
        for (local_10 = 0; (int)(uint)local_10 < param_4[1 = byte +0x4]; local_10 = local_10 + 1) {
          if (param_2 == *(int *)(*param_4 + (uint)local_10 * 4)) {
            return 0;
          }
        }
        iVar3 = FUN_00846dd0(iVar3,param_4);
        if (iVar3 == 0) {
          uVar4 = 0;
        }
        else if ((uint)param_3 == param_4[1 = byte +0x4] || (int)((uint)param_3 - param_4[1 = byte +0x4]) < 0) {
          uVar4 = 0;
        }
        else {
          uVar4 = 1;
        }
      }
      else {
        uVar4 = 0;
      }
    }
/* ... 6 more lines truncated by --max-lines */
```

---
## CARD C034
**CLAIM:** Function at 0x0090BD50 is `CSWGuiFeatsCharGen::RefreshGridStates`: repaint feat grid slot states: 0x89C220(3) then known->1 selected->4 choosable->0 granted->2.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0090BD50, size 570, header ret=0 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x0090BD50 (FUN_0090bd50)
string literals (0):
direct callees (10):
  0x00733700  CExoString::Constructor_5
  0x00733780  CExoString::~CExoString
  0x0090B9B0  (unnamed)
  0x0041FC30  (unnamed)
  0x0089C170  (unnamed)
  0x0089C070  (unnamed)
  0x0089C220  (unnamed)
  0x0089C290  (unnamed)
  0x00416E30  CSWGuiTextParams::SetText
  0x0041E870  (unnamed)
struct offsets touched (top 11 of [REG + 0xNN], EBP/ESP excluded):
  +0x1BD8 [dec 7128]  x4  via EAX,EDX
  +0x1BA8 [dec 7080]  x1  via ECX
  +0x1BAC [dec 7084]  x1  via ECX
  +0x1BB4 [dec 7092]  x1  via ECX
  +0x1BB8 [dec 7096]  x1  via ECX
  +0x1BC0 [dec 7104]  x1  via EDX
  +0x1BC4 [dec 7108]  x1  via EDX
  +0x1BCC [dec 7116]  x1  via EAX
  +0x1BD0 [dec 7120]  x1  via EAX
  +0x1BDE [dec 7134]  x1  via ECX
  +0x1C01 [dec 7169]  x1  via EDX

DECOMPILE (offsets annotated; truncated):
// FN 0090bd50 FUN_0090bd50 size=570 ret=0 callers=3 callees=10

void __fastcall FUN_0090bd50(int param_1)

{
  undefined2 uVar1;
  undefined1 local_1c [8];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a238;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0089c220(3);
  for (local_14 = 0; local_14 < *(int *)(param_1 + 0x1bac); local_14 = local_14 + 1) {
    FUN_0089c170(*(undefined2 *)(*(int *)(param_1 + 0x1ba8) + local_14 * 2),1);
  }
  for (local_14 = 0; local_14 < *(int *)(param_1 + 0x1bd0); local_14 = local_14 + 1) {
    FUN_0089c170(*(undefined2 *)(*(int *)(param_1 + 0x1bcc) + local_14 * 2),4);
  }
  for (local_14 = 0; local_14 < *(int *)(param_1 + 0x1bc4); local_14 = local_14 + 1) {
    FUN_0089c170(*(undefined2 *)(*(int *)(param_1 + 0x1bc0) + local_14 * 2),0);
  }
  for (local_14 = 0; local_14 < *(int *)(param_1 + 0x1bb8); local_14 = local_14 + 1) {
    FUN_0089c170(*(undefined2 *)(*(int *)(param_1 + 0x1bb4) + local_14 * 2),2);
  }
  if (*(short *)(param_1 + 0x1bd8) == -1) {
    FUN_0041fc30(param_1 + 0x1bf4,0,1,0);
    uVar1 = FUN_0089c290();
    *(undefined2 *)(param_1 + 0x1bd8) = uVar1;
  }
  FUN_0089c070(*(undefined2 *)(param_1 + 0x1bd8));
  FUN_0041e870(*(undefined1 *)(param_1 + 0x1c01),0);
  FUN_0090b9b0(*(undefined2 *)(param_1 + 0x1bd8));
  FUN_00733700(*(undefined1 *)(param_1 + 0x1bde));
  local_8 = 0;
  FUN_00416e30(local_1c);
  local_8 = 0xffffffff;
  FUN_00733780();
  ExceptionList = local_10;
  return;
}
```

---
## CARD C035
**CLAIM:** Hook site at 0x009099A7 inside the function at 0x009098D0: bytes `0f b7 4d e4 51`. At this point choosable-power loop after the 4 vector probes, before the 0x848660 args are pushed: id = word [ebp-0x1C], panel [ebp-0x68]. Consumed-exit (skip) target 0x00909B51 = jmp back to loop head (e9 c8 fd ff ff region); nothing pushed yet.

```
SITE WINDOW (>> = claimed address):
   0090998B  8b 4d 98           MOV ECX,dword ptr [EBP + -0x68]
   0090998E  81 c1 d4 1b 00 00  ADD ECX,0x1bd4
   00909994  e8 47 a9 b0 ff     CALL 0x004142e0
   00909999  83 f8 ff           CMP EAX,-0x1
   0090999C  0f 85 af 01 00 00  JNZ 0x00909b51
   009099A2  0f b7 4d e4        MOVZX ECX,word ptr [EBP + -0x1c]
   009099A6  51                 PUSH ECX
>> 009099A7  8b 4d 98           MOV ECX,dword ptr [EBP + -0x68]
   009099AA  81 c1 ec 1b 00 00  ADD ECX,0x1bec
   009099B0  e8 2b a9 b0 ff     CALL 0x004142e0
   009099B5  83 f8 ff           CMP EAX,-0x1
   009099B8  0f 85 93 01 00 00  JNZ 0x00909b51
   009099BE  8b 55 98           MOV EDX,dword ptr [EBP + -0x68]
   009099C1  81 c2 e0 1b 00 00  ADD EDX,0x1be0
   009099C7  52                 PUSH EDX

EXIT WINDOW:
   00909B43  8b 4d 98           MOV ECX,dword ptr [EBP + -0x68]
   00909B46  81 c1 d4 1b 00 00  ADD ECX,0x1bd4
   00909B4C  e8 0f 4f f3 ff     CALL 0x0083ea60
>> 00909B51  e9 c8 fd ff ff     JMP 0x0090991e
   00909B56  c7 45 fc ff ff ff ff MOV dword ptr [EBP + -0x4],0xffffffff
   00909B5D  8d 4d e8           LEA ECX,[EBP + -0x18]
   00909B60  e8 1b 9c e2 ff     CALL 0x00733780
```

---
## CARD C036
**CLAIM:** Function at 0x0090B200 is `CSWGuiFeatsCharGen::OnInputEvent`: vtable slot 15 event switch: 0x27/0x2D accept, 0x28/0x2E back, 0x29 click, 0x2A recommended, arrows.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0090B200, size 281, header ret=8 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x0090B200 (FUN_0090b200)
string literals (0):
direct callees (9):
  0x0090B890  (unnamed)
  0x0040EAA0  CSWGuiPanel::HandleInputEvent
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x0089C350  (unnamed)
  0x0090B9B0  (unnamed)
  0x00908A80  (unnamed)
  0x0090B360  (unnamed)
  0x0090B430  CSWGuiFeatsCharGen::OnAcceptButton
  0x0090B6B0  (unnamed)
struct offsets touched (top 4 of [REG + 0xNN], EBP/ESP excluded):
  +0x1C [dec 28]  x4  via EAX,ECX,EDX
  +0x3C [dec 60]  x2  via EAX,EDX
  +0x18B8 [dec 6328]  x2  via EAX,EDX
  +0x90B33C [dec 9483068]  x1  via EDX

DECOMPILE (offsets annotated; truncated):
// FN 0090b200 FUN_0090b200 size=281 ret=8 callers=0 callees=9

void __thiscall FUN_0090b200(int param_1,undefined4 param_2,int param_3)

{
  undefined4 uVar1;
  
  if (param_3 != 0) {
    switch(param_2) {
    case 0x27:
    case 0x2d:
      FUN_004122a0(0);
      FUN_0090b430();
      break;
    case 0x28:
    case 0x2e:
      FUN_004122a0(0);
      FUN_0090b360();
      break;
    case 0x29:
      FUN_004122a0(0);
      uVar1 = FUN_00908a80();
      FUN_0090b890(uVar1);
      break;
    case 0x2a:
      FUN_004122a0(0);
      FUN_0090b6b0();
      break;
    case 0x2f:
    case 0x30:
    case 0x31:
    case 0x32:
    case 0x3d:
    case 0x3e:
    case 0x3f:
    case 0x40:
      uVar1 = FUN_0089c350(param_2);
      FUN_0090b9b0(uVar1);
      break;
    case 0x39:
      (**(code **)(*(int *)(param_1 + 0x18b8) + 0x3c))(0x31,1);
      break;
    case 0x3a:
      (**(code **)(*(int *)(param_1 + 0x18b8) + 0x3c))(0x32,1);
    }
  }
  FUN_0040eaa0(param_2,param_3);
  return;
}
```

---
## CARD C037
**CLAIM:** Function at 0x0090B890 is `CSWGuiFeatsCharGen::OnRecommendedButton`: feats Recommended button: AddDefaultPackageFeats path (ignores grid filter).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0090B890, size 263, header ret=4 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x0090B890 (FUN_0090b890)
string literals (0):
direct callees (8):
  0x0090C120  (unnamed)
  0x0090BF90  (unnamed)
  0x0090BD50  (unnamed)
  0x0073F750  ClientApp::GetGuiInGame
  0x0075BB00  (unnamed)
  0x0090BCB0  (unnamed)
  0x0075CA30  (unnamed)
  0x00410530  CSWGuiManager::AddPanel
struct offsets touched (top 5 of [REG + 0xNN], EBP/ESP excluded):
  +0x4 [dec 4]  x1  via EDX
  +0x1C [dec 28]  x1  via ECX
  +0x70 [dec 112]  x1  via EDX
  +0xA0 [dec 160]  x1  via ECX
  +0x1BDE [dec 7134]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 0090b890 FUN_0090b890 size=263 ret=4 callers=2 callees=8

void __thiscall FUN_0090b890(int param_1,undefined2 param_2)

{
  int *piVar1;
  int iVar2;
  int local_c;
  undefined1 local_5;
  
  FUN_0090bcb0(param_2,&local_5);
  local_c = 0;
  switch(local_5) {
  case 0:
    if (*(char *)(param_1 + 0x1bde) == '\0') {
      local_c = 0xa622;
    }
    else {
      FUN_0090bf90(param_2);
      FUN_0090bd50();
    }
    break;
  case 1:
    FUN_0090c120(param_2);
    FUN_0090bd50();
    break;
  case 2:
    local_c = 0xa4c7;
    break;
  case 3:
    local_c = 0xa4c6;
    break;
  case 4:
    local_c = 0xa4c8;
  }
  if (local_c != 0) {
    iVar2 = FUN_0073f750();
    piVar1 = *(int **)(iVar2 + 0xa0);
    FUN_0075bb00(0);
    FUN_0075ca30(0,0,0);
    (**(code **)(*piVar1 + 0x70))(local_c);
    FUN_00410530(piVar1,1,1);
  }
  return;
}
```

---
## CARD C038
**CLAIM:** Function at 0x0089C170 is `CSWGuiLevelUpIconGrid::SelectSlotById`: finds the slot holding an id and selects it; silent no-op when absent.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0089C170, size 170, header ret=8 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x0089C170 (FUN_0089c170)
string literals (0):
direct callees (1):
  0x0089A160  (unnamed)
struct offsets touched (top 1 of [REG + 0xNN], EBP/ESP excluded):
  +0x4 [dec 4]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 0089c170 FUN_0089c170 size=170 ret=8 callers=2 callees=1

void __thiscall FUN_0089c170(int *param_1,int param_2,undefined1 param_3)

{
  uint local_18;
  uint local_c;
  int local_8;
  
  local_8 = 0;
  do {
    if (param_1[1 = byte +0x4] <= local_8) {
      return;
    }
    for (local_18 = 0; (int)local_18 < 3; local_18 = local_18 + 1) {
      if (*(int *)(*(int *)(*param_1 + local_8 * 4) + 0x108 + local_18 * 0xb4) == param_2) {
        local_c = local_18;
        goto LAB_0089c1ed;
      }
    }
    local_c = 0xffffffff;
LAB_0089c1ed:
    if (local_c != 0xffffffff) {
      FUN_0089a160(local_c & 0xff,param_3);
      return;
    }
    local_8 = local_8 + 1;
  } while( true );
}
```

---
## CARD C039
**CLAIM:** Function at 0x0090E850 is `CSWGuiFeatsCharGen::OnInputEvent`: vtable slot 15 event switch: 0x27/0x2D accept, 0x28/0x2E back, 0x29 click, 0x2A recommended, arrows.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0090E850, size 245, header ret=8 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x0090E850 (FUN_0090e850)
string literals (0):
direct callees (7):
  0x0090FBD0  (unnamed)
  0x0090EAC0  (unnamed)
  0x0040EAA0  CSWGuiPanel::HandleInputEvent
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x0090EDE0  (unnamed)
  0x0090E990  (unnamed)
  0x0090FA70  (unnamed)
struct offsets touched (top 4 of [REG + 0xNN], EBP/ESP excluded):
  +0x1C [dec 28]  x5  via EAX,ECX,EDX
  +0x3C [dec 60]  x2  via EAX,EDX
  +0x70 [dec 112]  x2  via EAX,EDX
  +0x90E968 [dec 9496936]  x1  via EDX

DECOMPILE (offsets annotated; truncated):
// FN 0090e850 FUN_0090e850 size=245 ret=8 callers=0 callees=7

void __thiscall FUN_0090e850(int param_1,undefined4 param_2,int param_3)

{
  if (param_3 != 0) {
    switch(param_2) {
    case 0x27:
    case 0x2d:
      FUN_004122a0(0);
      FUN_0090fa70();
      break;
    case 0x28:
    case 0x2e:
      FUN_004122a0(0);
      FUN_0090e990();
      break;
    case 0x2a:
      FUN_004122a0(0);
      FUN_0090fbd0();
      break;
    case 0x2f:
    case 0x3f:
      FUN_004122a0(1);
      FUN_0090eac0();
      break;
    case 0x30:
    case 0x40:
      FUN_004122a0(1);
      FUN_0090ede0();
      break;
    case 0x39:
      (**(code **)(*(int *)(param_1 + 0x70) + 0x3c))(0x31,1);
      break;
    case 0x3a:
      (**(code **)(*(int *)(param_1 + 0x70) + 0x3c))(0x32,1);
    }
  }
  FUN_0040eaa0(param_2,param_3);
  return;
}
```

---
## CARD C040
**CLAIM:** Function at 0x00908540 is `CSWGuiFeatsCharGen::InitFeatLists`: feats panel init: known/granted vectors, pick counts, then grid 0x89B600, choosable 0x90C230, repaint 0x90BD50.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x00908540, size 986, header ret=0 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x00908540 (FUN_00908540)
string literals (0):
direct callees (21):
  0x009098D0  (unnamed)
  0x00847F90  CSWCLevelUpStats::ComputeNumberKnownSpellsLeft
  0x006C1450  (unnamed)
  0x007337C0  CExoString::operator=
  0x0073F750  ClientApp::GetGuiInGame
  0x006C12D0  (unnamed)
  0x0084A640  (unnamed)
  0x00414280  (unnamed)
  0x0089CB60  (unnamed)
  0x00733780  CExoString::~CExoString
  0x00733C40  (unnamed)
  0x00847A70  CSWCLevelUpStats::AddGrantedPowers
  0x0049B840  (unnamed)
  0x00416EA0  (unnamed)
  0x00733540  CExoString::Constructor
  0x0083EA60  CExoArrayList<uint32>::Add
  0x00909360  (unnamed)
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x00845520  (unnamed)
  0x00476620  (unnamed)
  0x00845560  (unnamed)
struct offsets touched (top 11 of [REG + 0xNN], EBP/ESP excluded):
  +0x1BB4 [dec 7092]  x4  via EAX,ECX,EDX
  +0x104 [dec 260]  x2  via EAX
  +0x1BA8 [dec 7080]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via ECX
  +0x32 [dec 50]  x1  via ECX
  +0x68 [dec 104]  x1  via EAX
  +0xAC [dec 172]  x1  via EAX
  +0x310 [dec 784]  x1  via ECX
  +0x1BA9 [dec 7081]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 00908540 FUN_00908540 size=986 ret=0 callers=1 callees=21

void __fastcall FUN_00908540(int param_1)

{
  undefined1 uVar1;
  ushort uVar2;
  undefined4 uVar3;
  int iVar4;
  ushort uVar5;
  undefined1 local_58 [8];
  int local_50;
  undefined1 local_4c [8];
  undefined4 local_44;
  int local_40;
  int local_3c;
  int local_38;
  int local_30;
  int local_2c;
  byte local_25;
  undefined4 local_24;
  byte local_1d;
  undefined1 local_1c [8];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00980b20;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  local_14 = *(int *)(*(int *)(param_1 + 0x68) + 0x310);
  for (local_1d = 0; local_1d < *(byte *)(local_14 + 0x32); local_1d = local_1d + 1) {
    for (local_25 = 0; uVar5 = (ushort)local_25, uVar2 = FUN_00845560(local_1d,0), uVar5 < uVar2;
        local_25 = local_25 + 1) {
      local_24 = FUN_00845520(local_1d,0,local_25);
      local_2c = 0;
      local_30 = 0;
      FUN_006c1450(local_24);
      uVar3 = FUN_006c12d0(local_4c);
      local_8._0_1_ = 1;
      FUN_007337c0(uVar3);
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_00733780();
      while ((local_2c == 0 && (local_30 < *(int *)(param_1 + 0x1bcc)))) {
        FUN_006c1450(*(undefined4 *)(*(int *)(param_1 + 0x1bc8) + local_30 * 4));
/* ... 54 more lines truncated by --max-lines */
```

---
## CARD C041
**CLAIM:** Function at 0x0089C070 is `CSWGuiLevelUpIconGrid::SetSlotStateById`: finds the slot holding an id by scanning rows and sets its state; silent no-op when absent.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0089C070, size 250, header ret=4 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x0089C070 (FUN_0089c070)
string literals (0):
direct callees (1):
  0x0089A0E0  (unnamed)
struct offsets touched (top 3 of [REG + 0xNN], EBP/ESP excluded):
  +0xC [dec 12]  x4  via EAX,ECX,EDX
  +0xD [dec 13]  x4  via EAX,ECX,EDX
  +0x4 [dec 4]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 0089c070 FUN_0089c070 size=250 ret=4 callers=5 callees=1

void __thiscall FUN_0089c070(int *param_1,int param_2)

{
  uint local_18;
  uint local_c;
  uint local_8;
  
  local_8 = 0;
  do {
    if (param_1[1 = byte +0x4] <= (int)local_8) {
      return;
    }
    for (local_18 = 0; (int)local_18 < 3; local_18 = local_18 + 1) {
      if (*(int *)(*(int *)(*param_1 + local_8 * 4) + 0x108 + local_18 * 0xb4) == param_2) {
        local_c = local_18;
        goto LAB_0089c0f1;
      }
    }
    local_c = 0xffffffff;
LAB_0089c0f1:
    if (local_c != 0xffffffff) {
      if ((*(byte *)(param_1 + 0x3 [dec 3] {int*: byte +0xc}) != local_c) || (*(byte *)((int)param_1 + 0xd) != local_8)) {
        FUN_0089a0e0((char)param_1[3 = byte +0xc],0);
        *(undefined1 *)(param_1 + 0x3 [dec 3] {int*: byte +0xc}) = (undefined1)local_c;
        *(undefined1 *)((int)param_1 + 0xd) = (undefined1)local_8;
      }
      FUN_0089a0e0((char)param_1[3 = byte +0xc],1);
      return;
    }
    local_8 = local_8 + 1;
  } while( true );
}
```

---
## CARD C042
**CLAIM:** Function at 0x00908B50 is `CSWGuiPowersLevelUp::OnInputEvent`: vtable slot 15 event switch for powers panel.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x00908B50, size 368, header ret=0 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x00908B50 (FUN_00908b50)
string literals (0):
direct callees (8):
  0x009047F0  (unnamed)
  0x0073F750  ClientApp::GetGuiInGame
  0x0075BB00  (unnamed)
  0x004109C0  CSWGuiManager::PopModalPanel
  0x0075CA30  (unnamed)
  0x0040FE60  CSWGuiPanel::SetVisible
  0x00410530  CSWGuiManager::AddPanel
  0x008453E0  (unnamed)
struct offsets touched (top 13 of [REG + 0xNN], EBP/ESP excluded):
  +0x48 [dec 72]  x4  via EAX,EDX
  +0x1C [dec 28]  x2  via EAX
  +0x68 [dec 104]  x2  via EAX
  +0x6C [dec 108]  x2  via EAX,ECX
  +0x310 [dec 784]  x2  via ECX
  +0x4 [dec 4]  x1  via ECX
  +0x32 [dec 50]  x1  via EAX
  +0x70 [dec 112]  x1  via EAX
  +0xA0 [dec 160]  x1  via EAX
  +0x1BA8 [dec 7080]  x1  via EAX
  +0x1BD8 [dec 7128]  x1  via EDX
  +0x1BE0 [dec 7136]  x1  via EDX
  +0x1BE4 [dec 7140]  x1  via EDX

DECOMPILE (offsets annotated; truncated):
// FN 00908b50 FUN_00908b50 size=368 ret=0 callers=1 callees=8

void __fastcall FUN_00908b50(int param_1)

{
  char cVar1;
  int *piVar2;
  int iVar3;
  undefined4 local_c;
  
  if ((*(char *)(param_1 + 0x1ba8) == '\0') || (*(int *)(param_1 + 0x1bd8) < 1)) {
    cVar1 = *(char *)(*(int *)(*(int *)(param_1 + 0x68) + 0x310) + 0x32);
    for (local_c = 0; local_c < *(int *)(param_1 + 0x1be4); local_c = local_c + 1) {
      FUN_008453e0(cVar1 + -1,*(undefined4 *)(*(int *)(param_1 + 0x1be0) + local_c * 4));
    }
    FUN_004109c0();
    FUN_0040fe60(1);
    *(uint *)(param_1 + 0x48) = *(uint *)(param_1 + 0x48) & 0xffffff7f;
    *(uint *)(param_1 + 0x48) = *(uint *)(param_1 + 0x48) & 0xfffffcff | 0x200;
    FUN_009047f0();
  }
  else {
    iVar3 = FUN_0073f750();
    piVar2 = *(int **)(iVar3 + 0xa0);
    FUN_0075bb00(0);
    FUN_0075ca30(0,0,0);
    (**(code **)(*piVar2 + 0x70))(0x1eba5);
    FUN_00410530(piVar2,1,1);
  }
  return;
}
```

---
## CARD C043
**CLAIM:** Function at 0x0090EDE0 is `CSWGuiSkillsCharGen::RefreshRowStyling`: skills row dimming: disabled = !(skillRow+0x1C bit4), colour DAT_00A7FD4C vs DAT_00A7FDAC.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0090EDE0, size 2082, header ret=0 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x0090EDE0 (FUN_0090ede0)
string literals (0):
direct callees (20):
  0x00846350  (unnamed)
  0x007337C0  CExoString::operator=
  0x007335F0  CExoString::CExoString_copy
  0x0073F750  ClientApp::GetGuiInGame
  0x0075CA30  (unnamed)
  0x00416E30  CSWGuiTextParams::SetText
  0x0071EBD0  (unnamed)
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x00410530  CSWGuiManager::AddPanel
  0x00846690  (unnamed)
  0x00734F60  CExoString::CompareNoCase(char*)
  0x00733780  CExoString::~CExoString
  0x00733700  CExoString::Constructor_5
  0x00844860  (unnamed)
  0x0090F610  (unnamed)
  0x0071E910  (unnamed)
  0x0075BB00  (unnamed)
  0x00733540  CExoString::Constructor
  0x0071DEE0  (unnamed)
  0x009103A0  (unnamed)
struct offsets touched (top 15 of [REG + 0xNN], EBP/ESP excluded):
  +0x4CEC [dec 19692]  x14  via EAX,ECX,EDX
  +0x68 [dec 104]  x10  via EAX,ECX,EDX
  +0x310 [dec 784]  x8  via EAX,ECX,EDX
  +0x48 [dec 72]  x4  via EAX,ECX,EDX
  +0x90 [dec 144]  x3  via ECX,EDX
  +0x123 [dec 291]  x3  via EAX,EDX
  +0x4 [dec 4]  x2  via EDX
  +0x1C [dec 28]  x2  via ECX,EDX
  +0xA0 [dec 160]  x2  via ECX
  +0xB6 [dec 182]  x2  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 0090ede0 FUN_0090ede0 size=2082 ret=0 callers=1 callees=20

void __fastcall FUN_0090ede0(int param_1)

{
  int iVar1;
  uint uVar2;
  undefined1 *puStack_130;
  undefined *puStack_12c;
  uint uStack_128;
  undefined4 local_124;
  undefined4 local_120;
  undefined4 local_11c;
  undefined4 local_118;
  undefined4 local_114;
  undefined4 local_110;
  undefined4 local_10c;
  int local_108;
  int local_104;
  int local_100;
  char local_f9;
  ushort local_f8;
  byte local_f5;
  int local_f4;
  int local_f0;
  int local_ec;
  char local_e5;
  ushort local_e4;
  byte local_e1;
  int local_e0;
  int local_dc;
  undefined4 local_d8;
  ushort local_d2;
  int local_d0;
  undefined4 local_cc;
  int local_c8;
  uint local_c4;
  char local_be;
  byte local_bd;
  int local_bc;
  int local_b8;
  uint local_b4;
  undefined4 local_b0;
  int local_ac;
  uint local_a8;
  ushort local_a2;
  int local_a0;
  int local_9c;
  uint local_98;
/* ... 266 more lines truncated by --max-lines */
```

---
## CARD C044
**CLAIM:** Hook site at 0x0089B6F0 inside the function at 0x0089B600: bytes `89 8d 3c ff ff ff`. At this point loop body of feat grid builder right after lea ecx,[eax+edx*2]: ecx = &featId (ushort) of the current candidate; EAX and flags untouched. Consumed-exit (skip) target 0x0089BE96 = loop increment (mov edx,[ebp-0x28]); nothing pushed yet.

```
SITE WINDOW (>> = claimed address):
   0089B6CF  3b 8d 40 ff ff ff  CMP ECX,dword ptr [EBP + 0xffffff40]
   0089B6D5  0f 8d c9 07 00 00  JGE 0x0089bea4
   0089B6DB  8b 55 d8           MOV EDX,dword ptr [EBP + -0x28]
   0089B6DE  8b 45 dc           MOV EAX,dword ptr [EBP + -0x24]
   0089B6E1  8d 0c 50           LEA ECX,[EAX + EDX*0x2]
   0089B6E4  89 8d 3c ff ff ff  MOV dword ptr [EBP + 0xffffff3c],ECX
   0089B6EA  8b 95 3c ff ff ff  MOV EDX,dword ptr [EBP + 0xffffff3c]
>> 0089B6F0  0f b7 02           MOVZX EAX,word ptr [EDX]
   0089B6F3  50                 PUSH EAX
   0089B6F4  8b 0d d0 b4 a1 00  MOV ECX,dword ptr [0x00a1b4d0]
   0089B6FA  e8 f1 69 e0 ff     CALL 0x006a20f0
   0089B6FF  89 45 e8           MOV dword ptr [EBP + -0x18],EAX
   0089B702  8b 4d d8           MOV ECX,dword ptr [EBP + -0x28]
   0089B705  8b 55 dc           MOV EDX,dword ptr [EBP + -0x24]
   0089B708  0f b7 04 4a        MOVZX EAX,word ptr [EDX + ECX*0x2]

EXIT WINDOW:
   0089BE8A  51                 PUSH ECX
   0089BE8B  8b 8d fc fe ff ff  MOV ECX,dword ptr [EBP + 0xfffffefc]
   0089BE91  e8 3a 00 00 00     CALL 0x0089bed0
>> 0089BE96  8b 55 d8           MOV EDX,dword ptr [EBP + -0x28]
   0089BE99  83 c2 01           ADD EDX,0x1
   0089BE9C  89 55 d8           MOV dword ptr [EBP + -0x28],EDX
   0089BE9F  e9 15 f8 ff ff     JMP 0x0089b6b9
```


# BATCH 3 (cards C045-C066)

---
## CARD C045
**CLAIM:** Function at 0x0089AF30 is `CSWGuiLevelUpIconGrid::AddFeatRow`: allocs 0x324 grid row for (baseFeat, tier2, tier3); ids at row+0x108 + c*0xB4.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0089AF30, size 463, header ret=16 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x0089AF30 (FUN_0089af30)
string literals (0):
direct callees (9):
  0x00710EB0  (unnamed)
  0x007107F0  (unnamed)
  0x006C1450  (unnamed)
  0x009196EE  __security_check_cookie
  0x00416090  (unnamed)
  0x00899930  (unnamed)
  0x0083EA60  CExoArrayList<uint32>::Add
  0x00919723  operator new
  0x00710CA0  (unnamed)
struct offsets touched (top 2 of [REG + 0xNN], EBP/ESP excluded):
  +0x4 [dec 4]  x1  via EDX
  +0x104 [dec 260]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 0089af30 FUN_0089af30 size=463 ret=16 callers=1 callees=9

void FUN_0089af30(int param_1,undefined4 param_2,undefined4 param_3,int param_4)

{
  uint uVar1;
  int *piVar2;
  undefined4 extraout_var;
  undefined4 uVar3;
  undefined4 local_84;
  undefined4 local_80;
  undefined4 local_7c;
  undefined4 local_78;
  undefined1 local_74 [16];
  undefined1 local_64 [16];
  undefined1 local_54 [16];
  int local_44;
  int *local_40;
  int local_3c;
  undefined1 local_38 [16];
  uint local_28;
  int *local_24;
  int local_20[4 = byte +0x10];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00976b5b;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_28 = uVar1;
  FUN_007107f0(uVar1);
  uVar3 = extraout_var;
  local_44 = FUN_00919723(0x324);
  local_8 = 0;
  if (local_44 == 0) {
    piVar2 = (int *)0x0;
  }
  else {
    piVar2 = (int *)FUN_00899930();
  }
  local_8 = 0xffffffff;
  local_20[0] = param_1;
  local_20[1 = byte +0x4] = param_2;
  local_20[2 = byte +0x8] = param_3;
  local_40 = piVar2;
  local_24 = piVar2;
/* ... 26 more lines truncated by --max-lines */
```

---
## CARD C046
**CLAIM:** Function at 0x008468C0 is `CSWCLevelUpStats::IncrementSkillRank`: skill rank-up on the stats object: cap (level+3, halved cross-class), +1 rank via 0x845C10, deducts 1 or 2 points, returns bool.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x008468C0, size 437, header ret=8 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x008468C0 (FUN_008468c0)
string literals (0):
direct callees (5):
  0x00844E40  CSWSCreatureStats::GetClassByPosition
  0x006F24D0  (unnamed)
  0x00845C10  (unnamed)
  0x00844FF0  (unnamed)
  0x006F2440  (unnamed)
struct offsets touched (top 7 of [REG + 0xNN], EBP/ESP excluded):
  +0xB6 [dec 182]  x4  via EAX,ECX
  +0x90 [dec 144]  x2  via ECX,EDX
  +0x123 [dec 291]  x2  via EAX,EDX
  +0x1C [dec 28]  x1  via EDX
  +0x32 [dec 50]  x1  via EDX
  +0x124 [dec 292]  x1  via ECX
  +0x12C [dec 300]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 008468c0 FUN_008468c0 size=437 ret=8 callers=1 callees=5

void __thiscall FUN_008468c0(int param_1,ushort param_2,byte param_3)

{
  bool bVar1;
  byte bVar2;
  int iVar3;
  bool bVar4;
  char local_1e;
  char local_1c;
  uint local_c;
  
  bVar1 = false;
  iVar3 = (uint)param_2 * 0x20 + *(int *)(DAT_00a1b4d0 + 0x12c [dec 300]);
  if (iVar3 != 0) {
    bVar4 = (*(uint *)(iVar3 + 0x1c) >> 3 & 1) != 0;
    for (local_c = 0; (int)local_c < (int)(uint)*(byte *)(param_1 + 0x32); local_c = local_c + 1) {
      bVar2 = FUN_00844e40(local_c & 0xff);
      if (((uint)bVar2 * 0x238 + *(int *)(DAT_00a1b4d0 + 0x124) != 0) &&
         (iVar3 = FUN_006f24d0(param_2), iVar3 != 0)) {
        bVar4 = true;
        iVar3 = FUN_006f2440(param_2);
        if ((iVar3 != 0) || (iVar3 = FUN_00844ff0(param_2), iVar3 != 0)) {
          bVar1 = true;
          break;
        }
      }
    }
    if (bVar4) {
      if (param_2 < *(byte *)(DAT_00a1b4d0 + 0x123)) {
        local_1c = *(char *)((uint)param_2 + *(int *)(param_1 + 0x90));
      }
      else {
        local_1c = '\0';
      }
      if ((int)local_1c != (uint)param_3) {
        if (param_2 < *(byte *)(DAT_00a1b4d0 + 0x123)) {
          local_1e = *(char *)((uint)param_2 + *(int *)(param_1 + 0x90));
        }
        else {
          local_1e = '\0';
        }
        FUN_00845c10(param_2,local_1e + -1);
        if (bVar1) {
          *(short *)(param_1 + 0xb6) = *(short *)(param_1 + 0xb6) + 1;
        }
        else {
          *(short *)(param_1 + 0xb6) = *(short *)(param_1 + 0xb6) + 2;
/* ... 6 more lines truncated by --max-lines */
```

---
## CARD C047
**CLAIM:** Hook site at 0x009099A2 inside the function at 0x009098D0: bytes `0f b7 4d e4 51`. At this point choosable-power loop after the 4 vector probes, before the 0x848660 args are pushed: id = word [ebp-0x1C], panel [ebp-0x68]. Consumed-exit (skip) target 0x009099DB = jmp back to loop head (e9 c8 fd ff ff region); nothing pushed yet.

```
SITE WINDOW (>> = claimed address):
   00909986  0f b7 45 e4        MOVZX EAX,word ptr [EBP + -0x1c]
   0090998A  50                 PUSH EAX
   0090998B  8b 4d 98           MOV ECX,dword ptr [EBP + -0x68]
   0090998E  81 c1 d4 1b 00 00  ADD ECX,0x1bd4
   00909994  e8 47 a9 b0 ff     CALL 0x004142e0
   00909999  83 f8 ff           CMP EAX,-0x1
   0090999C  0f 85 af 01 00 00  JNZ 0x00909b51
>> 009099A2  0f b7 4d e4        MOVZX ECX,word ptr [EBP + -0x1c]
   009099A6  51                 PUSH ECX
   009099A7  8b 4d 98           MOV ECX,dword ptr [EBP + -0x68]
   009099AA  81 c1 ec 1b 00 00  ADD ECX,0x1bec
   009099B0  e8 2b a9 b0 ff     CALL 0x004142e0
   009099B5  83 f8 ff           CMP EAX,-0x1
   009099B8  0f 85 93 01 00 00  JNZ 0x00909b51
   009099BE  8b 55 98           MOV EDX,dword ptr [EBP + -0x68]

EXIT WINDOW:
   009099D3  0f b7 55 e4        MOVZX EDX,word ptr [EBP + -0x1c]
   009099D7  52                 PUSH EDX
   009099D8  8b 4d f0           MOV ECX,dword ptr [EBP + -0x10]
>> 009099DB  e8 80 ec f3 ff     CALL 0x00848660
   009099E0  85 c0              TEST EAX,EAX
   009099E2  0f 84 69 01 00 00  JZ 0x00909b51
   009099E8  a1 d0 b4 a1 00     MOV EAX,[0x00a1b4d0]
```

---
## CARD C048
**CLAIM:** Function at 0x008F4AF0 is `CSWGuiLevelUpPanel::Constructor`: level-up step panel ctor (+0x68 = live client creature).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x008F4AF0, size 4281, header ret=12 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x008F4AF0 (FUN_008f4af0)
string literals (38):
  0x009A96AC  at 008f4f27  'MAINCG_p'
  0x009A969C  at 008f4f47  'MAIN_TITLE_LBL'
  0x0099D72C  at 008f4f81  'LBL_NAME'
  0x009A9690  at 008f4fbe  'LBL_CLASS'
  0x009A9680  at 008f4ffb  'LBL_LEVEL_VAL'
  0x009A9678  at 008f5037  'STR_LBL'
  0x009A9670  at 008f5074  'DEX_LBL'
  0x009A9668  at 008f50b1  'CON_LBL'
  0x009A9660  at 008f50ed  'INT_LBL'
  0x009A9658  at 008f512a  'WIS_LBL'
  0x009A9650  at 008f5170  'CHA_LBL'
  0x009A9644  at 008f51b5  'STR_AB_LBL'
  0x009A9638  at 008f51fb  'DEX_AB_LBL'
  0x009A962C  at 008f5241  'CON_AB_LBL'
  0x009A9620  at 008f5286  'INT_AB_LBL'
  0x009A9614  at 008f52cc  'WIS_AB_LBL'
  0x009A9608  at 008f5312  'CHA_AB_LBL'
  0x009A9600  at 008f5357  'LBL_VIT'
  0x009A7504  at 008f539d  'LBL_DEF'
  0x009A95F0  at 008f53e3  'NEW_WILL_LBL'
  0x009A95E0  at 008f5428  'NEW_REFL_LBL'
  0x009A95D0  at 008f546e  'NEW_FORT_LBL'
  0x009A6E64  at 008f54b4  'LBL_BEVEL_L'
  0x009A6E58  at 008f54f9  'LBL_BEVEL_M'
  0x009A95C4  at 008f553f  'LBL_BEVEL_R'
  0x009A3C58  at 008f5585  'LBL_FORTITUDE'
  0x009A3C4C  at 008f55ca  'LBL_REFLEX'
  0x009A3C40  at 008f5610  'LBL_WILL'
  0x009A95B8  at 008f5656  'MODEL_LBL'
  0x009A95A8  at 008f569b  'PORTRAIT_LBL'
  0x009A9598  at 008f56e1  'LBL_STATSBACK'
  0x009A39DC  at 008f5727  'LBL_STATSBORDER'

DECOMPILE (offsets annotated; truncated):
// FN 008f4af0 FUN_008f4af0 size=4281 ret=12 callers=1 callees=26

undefined4 * __thiscall
FUN_008f4af0(undefined4 *param_1,undefined4 param_2,undefined4 param_3,int *param_4)

{
  uint uVar1;
  undefined4 uVar2;
  float10 fVar3;
  undefined4 local_1d4;
  undefined4 local_1d0;
  undefined4 local_1a8;
  undefined4 local_1a4;
  undefined4 local_1a0;
  undefined1 local_19c [8];
  undefined4 local_194;
  undefined4 local_190;
  undefined4 local_18c;
  undefined4 local_188;
  undefined4 local_184;
  undefined4 local_180;
  undefined4 local_17c;
  undefined1 local_178 [16];
  undefined1 local_168 [8];
  int local_160;
  undefined4 local_15c;
  undefined1 local_158 [8];
  undefined1 local_150 [8];
  undefined1 local_148 [8];
  undefined1 local_140 [8];
  undefined1 local_138 [8];
  undefined1 local_130 [8];
  undefined1 local_128 [8];
  undefined1 local_120 [8];
  undefined1 local_118 [8];
  undefined1 local_110 [8];
  undefined1 local_108 [8];
  undefined1 local_100 [8];
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
/* ... 365 more lines truncated by --max-lines */
```

---
## CARD C049
**CLAIM:** Function at 0x0089A760 is `CSWGuiLevelUpIconGrid::BuildSortedFeatList`: sorts feat ids by tier then id, caches at GUI state +0x103C.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0089A760, size 829, header ret=4 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x0089A760 (FUN_0089a760)
string literals (0):
direct callees (6):
  0x006C1450  (unnamed)
  0x0073F750  ClientApp::GetGuiInGame
  0x0040E120  (unnamed)
  0x0083EA60  CExoArrayList<uint32>::Add
  0x006C11C0  (unnamed)
  0x00414280  (unnamed)
struct offsets touched (top 4 of [REG + 0xNN], EBP/ESP excluded):
  +0x4 [dec 4]  x4  via EAX,EDX
  +0x104 [dec 260]  x3  via ECX,EDX
  +0x14C [dec 332]  x2  via EAX
  +0xC [dec 12]  x1  via EAX

DECOMPILE (offsets annotated; truncated):
// FN 0089a760 FUN_0089a760 size=829 ret=4 callers=1 callees=6

void FUN_0089a760(int *param_1)

{
  bool bVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  int iVar5;
  int local_14;
  int local_c;
  
  iVar2 = FUN_0073f750();
  if ((iVar2 + 0x1048 == 0) || (*(int *)(iVar2 + 0x104c) < 1)) {
    local_c = 0;
    while( true ) {
      if (**(int **)(DAT_00a1b4d0 + 0x104) <= local_c) break;
      iVar2 = FUN_006c1450(local_c);
      if ((iVar2 != 0) &&
         (((*(char *)(iVar2 + 0x14c) == '\x01' || (*(char *)(iVar2 + 0x14c) == '\x06')) &&
          (*(int *)(iVar2 + 0xc) != 0)))) {
        bVar1 = false;
        for (local_14 = 0; local_14 < param_1[1 = byte +0x4]; local_14 = local_14 + 1) {
          iVar2 = FUN_006c1450(*(undefined4 *)(*param_1 + local_14 * 4));
          if (iVar2 != 0) {
            iVar2 = FUN_006c11c0(0);
            uVar3 = FUN_006c11c0(0);
            uVar4 = FUN_006c11c0(0);
            iVar5 = FUN_006c11c0(0);
            if ((int)(iVar5 + (uVar4 / 10) * -10) < (int)(iVar2 + (uVar3 / 10) * -10)) {
              bVar1 = true;
            }
            else {
              iVar2 = FUN_006c11c0(0);
              uVar3 = FUN_006c11c0(0);
              uVar4 = FUN_006c11c0(0);
              iVar5 = FUN_006c11c0(0);
              if (iVar2 + (uVar3 / 10) * -10 == iVar5 + (uVar4 / 10) * -10) {
                iVar2 = FUN_006c11c0(0);
                iVar5 = FUN_006c11c0(0);
                if (iVar2 / 1000 < iVar5 / 1000) {
                  bVar1 = true;
                }
                else {
                  iVar2 = FUN_006c11c0(0);
                  iVar5 = FUN_006c11c0(0);
                  if (iVar2 / 1000 == iVar5 / 1000) {
                    iVar2 = FUN_006c11c0(0);
/* ... 31 more lines truncated by --max-lines */
```

---
## CARD C050
**CLAIM:** Call site at 0x009099D7 in the function at 0x009098D0: bytes `e8 80 ec f3 ff` = call to 0x00848660 [unnamed-in-DB power-availability test (candidate in this packet)]; 0x848660(S, spell, +0x1BA9, &selected +0x1BE0): 3 stack args pushed at 0x9099BE-0x9099D7.

```
CALL WINDOW (>> = claimed address):
   009099B8  0f 85 93 01 00 00  JNZ 0x00909b51
   009099BE  8b 55 98           MOV EDX,dword ptr [EBP + -0x68]
   009099C1  81 c2 e0 1b 00 00  ADD EDX,0x1be0
   009099C7  52                 PUSH EDX
   009099C8  8b 45 98           MOV EAX,dword ptr [EBP + -0x68]
   009099CB  0f b6 88 a9 1b 00 00 MOVZX ECX,byte ptr [EAX + 0x1ba9]
   009099D2  51                 PUSH ECX
   009099D3  0f b7 55 e4        MOVZX EDX,word ptr [EBP + -0x1c]
>> 009099D7  52                 PUSH EDX
   009099D8  8b 4d f0           MOV ECX,dword ptr [EBP + -0x10]
   009099DB  e8 80 ec f3 ff     CALL 0x00848660
   009099E0  85 c0              TEST EAX,EAX
   009099E2  0f 84 69 01 00 00  JZ 0x00909b51
```

---
## CARD C051
**CLAIM:** Class `CSWGuiFeatsCharGen` has vftable 0x009AA1AC; its constructor is 0x00902F60.

```
ASM lines of ctor 0x00902F60 containing a 0x9AAxxx constant:
00902faa  c701aca19a00          MOV dword ptr [ECX],0x9aa1ac
009030d8  6894a19a00            PUSH 0x9aa194
0090311b  688ca19a00            PUSH 0x9aa18c
0090317b  687ca19a00            PUSH 0x9aa17c
009031bd  6870a19a00            PUSH 0x9aa170

ANCHORS of ctor:
anchors for 0x00902F60 (FUN_00902f60)
string literals (5):
  0x009AA194  at 009030d8  'LEVELUPPNL_p'
  0x009AA18C  at 0090311b  'LBL_%d'
  0x009AA17C  at 0090317b  'BTN_STEPNAME%d'
  0x009AA170  at 009031bd  'LBL_NUM%d'
  0x0099E67C  at 00903239  'BTN_CANCEL'
direct callees (26):
  0x0041A510  (unnamed)
  0x00847E50  CSWCLevelUpStats::CalcNumberFeats
  0x004149F0  (unnamed)
  0x0091D67E  (unnamed)

VTABLE DUMP:
0x9AA1AC (28 slots)
  slot 0 +0x0 = 0x903AA0
  slot 15 +0x3C = 0x903ED0
  slot 18 +0x48 = 0x903BA0
```

---
## CARD C052
**CLAIM:** Function at 0x0090C120 is `CSWGuiFeatsCharGen::BuildChoosableFeatVector`: builds panel+0x1BC0 choosable feat vector, CanChooseFeat per id.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0090C120, size 271, header ret=4 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x0090C120 (FUN_0090c120)
string literals (0):
direct callees (7):
  0x0090C120  (unnamed)
  0x0090C670  (unnamed)
  0x005AA2C0  (unnamed)
  0x00846F00  (unnamed)
  0x0090C230  (unnamed)
  0x0090C620  (unnamed)
  0x006A20F0  (unnamed)
struct offsets touched (top 5 of [REG + 0xNN], EBP/ESP excluded):
  +0x1BCC [dec 7116]  x2  via EAX,ECX
  +0x1BDE [dec 7134]  x2  via ECX,EDX
  +0x68 [dec 104]  x1  via EDX
  +0x310 [dec 784]  x1  via EAX
  +0x1BD0 [dec 7120]  x1  via EAX

DECOMPILE (offsets annotated; truncated):
// FN 0090c120 FUN_0090c120 size=271 ret=4 callers=3 callees=7

void __thiscall FUN_0090c120(int param_1,undefined2 param_2)

{
  bool bVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 local_10;
  
  iVar2 = FUN_0090c620(param_2);
  if (iVar2 != -1) {
    FUN_005aa2c0(iVar2);
    *(char *)(param_1 + 0x1bde) = *(char *)(param_1 + 0x1bde) + '\x01';
    bVar1 = true;
    for (local_10 = 0; local_10 < *(int *)(param_1 + 0x1bd0); local_10 = local_10 + 1) {
      iVar2 = param_1 + 0x1bcc;
      uVar3 = FUN_006a20f0(*(undefined2 *)(*(int *)(param_1 + 0x1bcc) + local_10 * 2));
      iVar2 = FUN_00846f00(uVar3,iVar2);
      if (iVar2 == 0) {
        FUN_0090c120(*(undefined2 *)(*(int *)(param_1 + 0x1bcc) + local_10 * 2));
        bVar1 = false;
        break;
      }
    }
    if (bVar1) {
      FUN_0090c670(0);
      FUN_0090c230();
    }
  }
  return;
}
```

---
## CARD C053
**CLAIM:** Function at 0x00909E00 is `CSWGuiFeatsCharGen::Constructor`: feats level-up panel ctor (size 0x1C08, grid ptr at +0x1BF4).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x00909E00, size 2914, header ret=16 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x00909E00 (FUN_00909e00)
string literals (14):
  0x009AA424  at 0090a0d2  'FTCHRGEN_p'
  0x009A969C  at 0090a0f2  'MAIN_TITLE_LBL'
  0x009AA32C  at 0090a12c  'SUB_TITLE_LBL'
  0x009AA318  at 0090a169  'REMAINING_BOX_LBL'
  0x009AA404  at 0090a1a5  'STD_REMAINING_SELECTIONS_LBL'
  0x009AA3E4  at 0090a1e2  'STD_SELECTIONS_REMAINING_LBL'
  0x009A1810  at 0090a21f  'LB_DESC'
  0x009AA3D8  at 0090a268  'LB_FEATS'
  0x0099D72C  at 0090a2a4  'LBL_NAME'
  0x009A6968  at 0090a2e1  'BTN_ACCEPT'
  0x009AA3C8  at 0090a327  'BTN_RECOMMENDED'
  0x009AA3BC  at 0090a3b2  'BTN_SELECT'
  0x0099E548  at 0090a3f8  'LBL_BAR1'
  0x0099E53C  at 0090a43d  'LBL_BAR2'
direct callees (25):
  0x0041A510  (unnamed)
  0x0091D67E  (unnamed)
  0x00760EA0  (unnamed)
  0x00733DF0  CExoString::operator+
  0x005933B0  (unnamed)
  0x0091C860  _ftol2_sse
  0x0041CE90  (unnamed)
  0x00419740  (unnamed)
  0x0040E5A0  CSWGuiPanel::Constructor
  0x007337C0  CExoString::operator=
  0x0090AEA0  (unnamed)
  0x0040F5A0  CSWGuiPanel::StopLoadFromLayout
  0x00733780  CExoString::~CExoString
  0x00733570  CExoString::CExoString_char
  0x00416EA0  (unnamed)
  0x0040F620  CSWGuiPanel::InitControl
  0x00418AF0  CSWGuiControl::AddEvent

DECOMPILE (offsets annotated; truncated):
// FN 00909e00 FUN_00909e00 size=2914 ret=16 callers=3 callees=25

undefined4 * __thiscall
FUN_00909e00(undefined4 *param_1,undefined4 param_2,undefined4 param_3,uint param_4,uint param_5)

{
  uint uVar1;
  undefined4 uVar2;
  float10 fVar3;
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
  undefined1 local_40 [16];
  int local_30;
  float local_2c;
  undefined1 local_28 [8];
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00980e9e;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_0040e5a0(param_2);
/* ... 202 more lines truncated by --max-lines */
```

---
## CARD C054
**CLAIM:** Class `CSWGuiFeatsCharGen` has vftable 0x009AA34C; its constructor is 0x009074E0.

```
ASM lines of ctor 0x009074E0 containing a 0x9AAxxx constant:
0090752a  c7014ca39a00          MOV dword ptr [ECX],0x9aa34c
009077e9  683ca39a00            PUSH 0x9aa33c
00907843  682ca39a00            PUSH 0x9aa32c
00907880  6818a39a00            PUSH 0x9aa318
009078bd  68fca29a00            PUSH 0x9aa2fc
009078f9  68e0a29a00            PUSH 0x9aa2e0

ANCHORS of ctor:
anchors for 0x009074E0 (FUN_009074e0)
string literals (15):
  0x009AA33C  at 009077e9  'pwrlvlup_p'
  0x009A969C  at 00907809  'MAIN_TITLE_LBL'
  0x009AA32C  at 00907843  'SUB_TITLE_LBL'
  0x009AA318  at 00907880  'REMAINING_BOX_LBL'
  0x009AA2FC  at 009078bd  'REMAINING_SELECTIONS_LBL'
  0x009AA2E0  at 009078f9  'SELECTIONS_REMAINING_LBL'
  0x009A1810  at 00907936  'LB_DESC'
  0x009AA2D4  at 00907980  'LB_POWERS'
  0x009AA2C8  at 009079bd  'LBL_POWER'
  0x009AA2B8  at 009079f9  'RECOMMENDED_BTN'

VTABLE DUMP:
0x9AA34C (28 slots)
  slot 0 +0x0 = 0x908040
  slot 15 +0x3C = 0x908920
  slot 18 +0x48 = 0x908460
```

---
## CARD C055
**CLAIM:** Function at 0x00902F60 is `CSWGuiLevelUpCharGen::Constructor`: level-up outer panel ctor (+0x68 temp baseline creature, +0x2CB4 creature id).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x00902F60, size 2867, header ret=8 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x00902F60 (FUN_00902f60)
string literals (5):
  0x009AA194  at 009030d8  'LEVELUPPNL_p'
  0x009AA18C  at 0090311b  'LBL_%d'
  0x009AA17C  at 0090317b  'BTN_STEPNAME%d'
  0x009AA170  at 009031bd  'LBL_NUM%d'
  0x0099E67C  at 00903239  'BTN_CANCEL'
direct callees (26):
  0x0041A510  (unnamed)
  0x00847E50  CSWCLevelUpStats::CalcNumberFeats
  0x004149F0  (unnamed)
  0x0091D67E  (unnamed)
  0x0084A640  (unnamed)
  0x008470A0  (unnamed)
  0x00760EA0  (unnamed)
  0x00733700  CExoString::Constructor_5
  0x00846200  (unnamed)
  0x00734270  CExoString::Format
  0x00847F90  CSWCLevelUpStats::ComputeNumberKnownSpellsLeft
  0x0040E5A0  CSWGuiPanel::Constructor
  0x0073F550  CClientExoApp::GetCreatureByGameObjectID
  0x00416E30  CSWGuiTextParams::SetText
  0x0040F5A0  CSWGuiPanel::StopLoadFromLayout
  0x00733780  CExoString::~CExoString
  0x00844860  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x00847A70  CSWCLevelUpStats::AddGrantedPowers
  0x0040F620  CSWGuiPanel::InitControl
  0x00418AF0  CSWGuiControl::AddEvent
  0x00733540  CExoString::Constructor
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x00710840  (unnamed)
  0x0073FEA0  CClientExoApp::GetSWGuiManager
  0x0040EE40  CSWGuiPanel::StartLoadFromLayout

DECOMPILE (offsets annotated; truncated):
// FN 00902f60 FUN_00902f60 size=2867 ret=8 callers=1 callees=26

/* WARNING: Removing unreachable block (ram,0x009032ab) */

undefined4 * __thiscall FUN_00902f60(undefined4 *param_1,undefined4 param_2,undefined4 param_3)

{
  uint uVar1;
  undefined4 uVar2;
  undefined1 local_90 [8];
  undefined1 local_88 [8];
  undefined1 local_80 [8];
  undefined1 local_78 [8];
  undefined1 local_70 [8];
  undefined1 local_68 [8];
  undefined1 local_60 [8];
  undefined1 local_58 [16];
  undefined1 local_48 [4];
  int local_44;
  undefined1 local_3c [4];
  int local_38;
  int local_30;
  byte local_29;
  uint local_28;
  undefined1 local_24 [8];
  int local_1c;
  uint local_18;
  byte local_11;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0097fc6b;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_0040e5a0(param_2);
  local_8 = 0;
  *param_1 = CSWGuiLevelUpPanel::vftable;
  _eh_vector_constructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable](param_1 + 0x1c,0x1c0,5,FUN_00758960,FUN_00420450);
  local_8._0_1_ = 1;
  _eh_vector_constructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable]
            (param_1 + 0x24c,0x148,5,FUN_00419740,
             CMFCToolBarButtonsListButton::~CMFCToolBarButtonsListButton);
  local_8._0_1_ = 2;
  _eh_vector_constructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable](param_1 + 0x3e6,0x1d0,5,FUN_0041a510,FUN_0041a3d0);
  local_8._0_1_ = 3;
  FUN_0041a510(uVar1);
/* ... 147 more lines truncated by --max-lines */
```

---
## CARD C056
**CLAIM:** Function at 0x0089BED0 is `CSWGuiLevelUpIconGrid::AddPowerRow`: allocs grid row for (spell, tier2, tier3, flag); power twin of 0x89BED0.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0089BED0, size 403, header ret=12 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x0089BED0 (FUN_0089bed0)
string literals (0):
direct callees (11):
  0x00710EB0  (unnamed)
  0x007107F0  (unnamed)
  0x009196EE  __security_check_cookie
  0x00416090  (unnamed)
  0x0091C860  _ftol2_sse
  0x00899930  (unnamed)
  0x0083EA60  CExoArrayList<uint32>::Add
  0x00919723  operator new
  0x00710CA0  (unnamed)
  0x00479AB0  (unnamed)
  0x006A20F0  (unnamed)
struct offsets touched (top 1 of [REG + 0xNN], EBP/ESP excluded):
  +0x4 [dec 4]  x1  via EAX

DECOMPILE (offsets annotated; truncated):
// FN 0089bed0 FUN_0089bed0 size=403 ret=12 callers=2 callees=11

void FUN_0089bed0(int param_1,undefined4 param_2,undefined4 param_3)

{
  uint uVar1;
  int *piVar2;
  float10 fVar3;
  undefined4 extraout_var;
  undefined4 uVar4;
  undefined4 local_78;
  undefined4 local_74;
  undefined4 local_70;
  undefined4 local_6c;
  undefined1 local_68 [16];
  undefined1 local_58 [16];
  int local_48;
  int *local_44;
  undefined1 local_40 [16];
  uint local_30;
  float local_2c;
  int *local_28;
  int local_24[5 = byte +0x14];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00976c7b;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_30 = uVar1;
  FUN_007107f0(uVar1);
  uVar4 = extraout_var;
  local_48 = FUN_00919723(0x324);
  local_8 = 0;
  if (local_48 == 0) {
    piVar2 = (int *)0x0;
  }
  else {
    piVar2 = (int *)FUN_00899930();
  }
  local_8 = 0xffffffff;
  local_24[0] = param_1;
  local_24[1 = byte +0x4] = param_2;
  local_24[2 = byte +0x8] = param_3;
  local_44 = piVar2;
  local_28 = piVar2;
/* ... 23 more lines truncated by --max-lines */
```

---
## CARD C057
**CLAIM:** Function at 0x0090E850 is `CSWGuiSkillsCharGen::OnInputEvent`: vtable slot 15 event switch for skills panel (R10 text says 0x90E860, vtable says 0x90E850).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0090E850, size 245, header ret=8 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x0090E850 (FUN_0090e850)
string literals (0):
direct callees (7):
  0x0090FBD0  (unnamed)
  0x0090EAC0  (unnamed)
  0x0040EAA0  CSWGuiPanel::HandleInputEvent
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x0090EDE0  (unnamed)
  0x0090E990  (unnamed)
  0x0090FA70  (unnamed)
struct offsets touched (top 4 of [REG + 0xNN], EBP/ESP excluded):
  +0x1C [dec 28]  x5  via EAX,ECX,EDX
  +0x3C [dec 60]  x2  via EAX,EDX
  +0x70 [dec 112]  x2  via EAX,EDX
  +0x90E968 [dec 9496936]  x1  via EDX

DECOMPILE (offsets annotated; truncated):
// FN 0090e850 FUN_0090e850 size=245 ret=8 callers=0 callees=7

void __thiscall FUN_0090e850(int param_1,undefined4 param_2,int param_3)

{
  if (param_3 != 0) {
    switch(param_2) {
    case 0x27:
    case 0x2d:
      FUN_004122a0(0);
      FUN_0090fa70();
      break;
    case 0x28:
    case 0x2e:
      FUN_004122a0(0);
      FUN_0090e990();
      break;
    case 0x2a:
      FUN_004122a0(0);
      FUN_0090fbd0();
      break;
    case 0x2f:
    case 0x3f:
      FUN_004122a0(1);
      FUN_0090eac0();
      break;
    case 0x30:
    case 0x40:
      FUN_004122a0(1);
      FUN_0090ede0();
      break;
    case 0x39:
      (**(code **)(*(int *)(param_1 + 0x70) + 0x3c))(0x31,1);
      break;
    case 0x3a:
      (**(code **)(*(int *)(param_1 + 0x70) + 0x3c))(0x32,1);
    }
  }
  FUN_0040eaa0(param_2,param_3);
  return;
}
```

---
## CARD C058
**CLAIM:** Function at 0x009074E0 is `CSWGuiSkillsCharGen::Constructor`: skills level-up panel ctor (8 fixed skill rows).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x009074E0, size 2903, header ret=8 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x009074E0 (FUN_009074e0)
string literals (15):
  0x009AA33C  at 009077e9  'pwrlvlup_p'
  0x009A969C  at 00907809  'MAIN_TITLE_LBL'
  0x009AA32C  at 00907843  'SUB_TITLE_LBL'
  0x009AA318  at 00907880  'REMAINING_BOX_LBL'
  0x009AA2FC  at 009078bd  'REMAINING_SELECTIONS_LBL'
  0x009AA2E0  at 009078f9  'SELECTIONS_REMAINING_LBL'
  0x009A1810  at 00907936  'LB_DESC'
  0x009AA2D4  at 00907980  'LB_POWERS'
  0x009AA2C8  at 009079bd  'LBL_POWER'
  0x009AA2B8  at 009079f9  'RECOMMENDED_BTN'
  0x009AA2AC  at 00907a36  'SELECT_BTN'
  0x009AA2A0  at 00907a73  'ACCEPT_BTN'
  0x009AA294  at 00907aaf  'BACK_BTN'
  0x0099E548  at 00907af5  'LBL_BAR1'
  0x0099E53C  at 00907b3b  'LBL_BAR2'
direct callees (25):
  0x0041A510  (unnamed)
  0x0091D67E  (unnamed)
  0x0084A640  (unnamed)
  0x00760EA0  (unnamed)
  0x00733DF0  CExoString::operator+
  0x0041CE90  (unnamed)
  0x00419740  (unnamed)
  0x0040E5A0  CSWGuiPanel::Constructor
  0x007337C0  CExoString::operator=
  0x0073F550  CClientExoApp::GetCreatureByGameObjectID
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x00908540  (unnamed)
  0x0040F5A0  CSWGuiPanel::StopLoadFromLayout
  0x00733780  CExoString::~CExoString
  0x00733570  CExoString::CExoString_char
  0x0040F620  CSWGuiPanel::InitControl

DECOMPILE (offsets annotated; truncated):
// FN 009074e0 FUN_009074e0 size=2903 ret=8 callers=1 callees=25

undefined4 * __thiscall FUN_009074e0(undefined4 *param_1,undefined4 param_2,undefined4 param_3)

{
  uint uVar1;
  undefined4 uVar2;
  int iVar3;
  undefined1 local_dc [8];
  undefined1 local_d4 [8];
  undefined1 local_cc [8];
  undefined1 local_c4 [8];
  undefined1 local_bc [8];
  undefined1 local_b4 [8];
  undefined1 local_ac [8];
  undefined1 local_a4 [8];
  undefined1 local_9c [8];
  undefined1 local_94 [8];
  undefined1 local_8c [8];
  undefined1 local_84 [8];
  undefined1 local_7c [8];
  undefined1 local_74 [8];
  undefined1 local_6c [8];
  undefined1 local_64 [8];
  undefined1 local_5c [8];
  undefined1 local_54 [8];
  undefined1 local_4c [8];
  undefined1 local_44 [8];
  undefined1 local_3c [8];
  undefined1 local_34 [8];
  undefined1 local_2c [16];
  int local_1c;
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009808b5;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_0040e5a0(param_2);
  local_8 = 0;
  *param_1 = CSWGuiPowersLevelUp::vftable;
  FUN_00419740(uVar1);
  local_8._0_1_ = 1;
  FUN_00419740();
  local_8._0_1_ = 2;
/* ... 186 more lines truncated by --max-lines */
```

---
## CARD C059
**CLAIM:** Function at 0x0089B600 is `CSWGuiLevelUpIconGrid::BuildPowerRows`: builds power icon grid from sorted spell list, rows via 0x89AF30 (0x89ADD4, 0x89AEB3).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0089B600, size 2244, header ret=8 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x0089B600 (FUN_0089b600)
string literals (7):
  0x00995148  at 0089b886  'handmaiden'
  0x00995164  at 0089b8f7  'baodur'
  0x00995188  at 0089b98c  'kreia'
  0x0099511C  at 0089b9fa  'hanharr'
  0x0099516C  at 0089bb5f  'atton'
  0x00995164  at 0089bc68  'baodur'
  0x00995188  at 0089bd27  'kreia'
direct callees (15):
  0x0089BED0  (unnamed)
  0x006F2690  (unnamed)
  0x006F2710  (unnamed)
  0x0089B100  (unnamed)
  0x006F2610  (unnamed)
  0x006F2830  (unnamed)
  0x006A20F0  (unnamed)
  0x0084A1D0  (unnamed)
  0x00734F60  CExoString::CompareNoCase(char*)
  0x00733780  CExoString::~CExoString
  0x00844E40  CSWSCreatureStats::GetClassByPosition
  0x005933B0  (unnamed)
  0x00847A30  (unnamed)
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x0089A6E0  (unnamed)
struct offsets touched (top 11 of [REG + 0xNN], EBP/ESP excluded):
  +0x49 [dec 73]  x3  via EAX,ECX
  +0x38 [dec 56]  x2  via ECX
  +0x4E [dec 78]  x2  via ECX,EDX
  +0x32 [dec 50]  x1  via EAX
  +0x3A [dec 58]  x1  via EAX
  +0x48 [dec 72]  x1  via ECX
  +0x4A [dec 74]  x1  via ECX
  +0x4B [dec 75]  x1  via EAX

DECOMPILE (offsets annotated; truncated):
// FN 0089b600 FUN_0089b600 size=2244 ret=8 callers=1 callees=15

void FUN_0089b600(int param_1,int param_2)

{
  byte bVar1;
  uint uVar2;
  int iVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined1 local_b8 [11];
  char local_ad;
  undefined1 local_ac [11];
  char local_a1;
  undefined1 local_a0 [8];
  int local_98;
  undefined1 local_94 [8];
  int local_8c;
  undefined1 local_88 [8];
  int local_80;
  undefined1 local_7c [8];
  int local_74;
  undefined1 local_70 [8];
  int local_68;
  undefined1 local_64 [8];
  int local_5c;
  undefined1 local_58 [8];
  int local_50;
  int local_4c;
  int local_48;
  int local_44;
  undefined1 local_3d;
  int local_3c;
  uint local_38;
  int local_34;
  uint local_30;
  int local_2c;
  int local_28;
  int local_24;
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
/* ... 183 more lines truncated by --max-lines */
```

---
## CARD C060
**CLAIM:** Hook site at 0x0089B6E4 inside the function at 0x0089B600: bytes `89 8d 3c ff ff ff`. At this point loop body of feat grid builder right after lea ecx,[eax+edx*2]: ecx = &featId (ushort) of the current candidate; EAX and flags untouched. Consumed-exit (skip) target 0x0089BE91 = loop increment (mov edx,[ebp-0x28]); nothing pushed yet.

```
SITE WINDOW (>> = claimed address):
   0089B6C6  89 85 40 ff ff ff  MOV dword ptr [EBP + 0xffffff40],EAX
   0089B6CC  8b 4d d8           MOV ECX,dword ptr [EBP + -0x28]
   0089B6CF  3b 8d 40 ff ff ff  CMP ECX,dword ptr [EBP + 0xffffff40]
   0089B6D5  0f 8d c9 07 00 00  JGE 0x0089bea4
   0089B6DB  8b 55 d8           MOV EDX,dword ptr [EBP + -0x28]
   0089B6DE  8b 45 dc           MOV EAX,dword ptr [EBP + -0x24]
   0089B6E1  8d 0c 50           LEA ECX,[EAX + EDX*0x2]
>> 0089B6E4  89 8d 3c ff ff ff  MOV dword ptr [EBP + 0xffffff3c],ECX
   0089B6EA  8b 95 3c ff ff ff  MOV EDX,dword ptr [EBP + 0xffffff3c]
   0089B6F0  0f b7 02           MOVZX EAX,word ptr [EDX]
   0089B6F3  50                 PUSH EAX
   0089B6F4  8b 0d d0 b4 a1 00  MOV ECX,dword ptr [0x00a1b4d0]
   0089B6FA  e8 f1 69 e0 ff     CALL 0x006a20f0
   0089B6FF  89 45 e8           MOV dword ptr [EBP + -0x18],EAX
   0089B702  8b 4d d8           MOV ECX,dword ptr [EBP + -0x28]

EXIT WINDOW:
   0089BE86  0f b7 0c 50        MOVZX ECX,word ptr [EAX + EDX*0x2]
   0089BE8A  51                 PUSH ECX
   0089BE8B  8b 8d fc fe ff ff  MOV ECX,dword ptr [EBP + 0xfffffefc]
>> 0089BE91  e8 3a 00 00 00     CALL 0x0089bed0
   0089BE96  8b 55 d8           MOV EDX,dword ptr [EBP + -0x28]
   0089BE99  83 c2 01           ADD EDX,0x1
   0089BE9C  89 55 d8           MOV dword ptr [EBP + -0x28],EDX
```

---
## CARD C061
**CLAIM:** Function at 0x0089B320 is `CSWGuiLevelUpIconGrid::BuildFeatRows`: builds feat icon grid from sorted feat list, rows via 0x89BED0 (call 0x89BE91).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0089B320, size 736, header ret=4 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x0089B320 (FUN_0089b320)
string literals (0):
direct callees (8):
  0x0089BED0  (unnamed)
  0x005933B0  (unnamed)
  0x0089B100  (unnamed)
  0x006F2830  (unnamed)
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x0089A6E0  (unnamed)
  0x006B83F0  CSWSCreatureStats::HasFeat
  0x006A20F0  (unnamed)
struct offsets touched (top 4 of [REG + 0xNN], EBP/ESP excluded):
  +0x38 [dec 56]  x2  via EAX,EDX
  +0x8D [dec 141]  x2  via EAX,ECX
  +0x3A [dec 58]  x1  via EDX
  +0x124 [dec 292]  x1  via EDX

DECOMPILE (offsets annotated; truncated):
// FN 0089b320 FUN_0089b320 size=736 ret=4 callers=1 callees=8

void FUN_0089b320(void)

{
  bool bVar1;
  uint uVar2;
  int iVar3;
  undefined4 extraout_var;
  int local_3c;
  uint local_34;
  uint local_2c;
  int local_28;
  int local_24;
  int local_20;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00976b98;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_005933b0(0);
  local_8 = 0;
  FUN_0089a6e0(uVar2,extraout_var);
  FUN_0089b100(&local_24);
  bVar1 = true;
  for (local_28 = 0; (bVar1 && (local_28 < local_20)); local_28 = local_28 + 1) {
    local_18 = FUN_006a20f0(*(undefined2 *)(local_24 + local_28 * 2));
    if ((*(short *)(local_24 + local_28 * 2) != 0xcf) &&
       ((*(short *)(local_24 + local_28 * 2) != 0xee && (local_18 != 0)))) {
      if (*(short *)(local_18 + 0x38) == -1) {
        iVar3 = FUN_006b83f0(*(undefined2 *)(local_24 + local_28 * 2));
        if (iVar3 != 0) {
          local_34 = 0xffffffff;
          local_2c = 0xffffffff;
          local_3c = local_28;
          while ((local_3c = local_3c + 1, local_3c < local_20 &&
                 ((local_34 == 0xffffffff || (local_2c == 0xffffffff))))) {
            iVar3 = FUN_006b83f0(*(undefined2 *)(local_24 + local_3c * 2));
            if ((iVar3 != 0) &&
               (((local_14 = FUN_006a20f0(*(undefined2 *)(local_24 + local_3c * 2)), local_14 != 0
                 && (*(short *)(local_14 + 0x38) == *(short *)(local_24 + local_28 * 2))) &&
                ((iVar3 = FUN_006f2830(*(undefined2 *)(local_24 + local_3c * 2)), iVar3 != 0 ||
                 (iVar3 = FUN_006b83f0(*(undefined2 *)(local_24 + local_3c * 2)), iVar3 != 0)))))) {
/* ... 21 more lines truncated by --max-lines */
```

---
## CARD C062
**CLAIM:** Function at 0x0089A160 is `CSWGuiLevelUpIconGrid::SetAllSlotStates`: sets every slot of every row to the given state (called with 3 = dimmed).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0089A160, size 999, header ret=8 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x0089A160 (FUN_0089a160)
string literals (0):
direct callees (0):
struct offsets touched (top 3 of [REG + 0xNN], EBP/ESP excluded):
  +0x4 [dec 4]  x11  via EAX,ECX,EDX
  +0x8 [dec 8]  x11  via EAX,ECX,EDX
  +0x24 [dec 36]  x4  via EAX,ECX,EDX

DECOMPILE (offsets annotated; truncated):
// FN 0089a160 FUN_0089a160 size=999 ret=8 callers=3 callees=0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0089a160(int param_1,byte param_2,undefined1 param_3)

{
  undefined4 *puVar1;
  int iVar2;
  
  *(undefined1 *)(param_1 + 0x10c + (uint)param_2 * 0xb4) = param_3;
  *(undefined4 *)(param_1 + 0x9c + (uint)param_2 * 0xb4) = 0x3f800000;
  *(undefined4 *)(param_1 + 0xf0 + (uint)param_2 * 0xb4) = 0x3f800000;
  if (param_2 != 0) {
    *(undefined4 *)(param_1 + 0x2b8 + (param_2 - 1) * 0x54) = 0x3f800000;
  }
  switch(param_3) {
  case 0:
    *(undefined4 *)(param_1 + 0x9c + (uint)param_2 * 0xb4) = _DAT_0098be34;
    if ((*(uint *)(param_1 + 0x110 + (uint)param_2 * 0xb4) & 1) == 0) {
      puVar1 = (undefined4 *)(param_1 + 0xf4 + (uint)param_2 * 0xb4);
      *puVar1 = DAT_00a7fde8;
      puVar1[1 = byte +0x4] = DAT_00a7fdec;
      puVar1[2 = byte +0x8] = DAT_00a7fdf0;
    }
    else {
      puVar1 = (undefined4 *)(param_1 + 0xf4 + (uint)param_2 * 0xb4);
      *puVar1 = DAT_00a7fde8;
      puVar1[1 = byte +0x4] = DAT_00a7fdec;
      puVar1[2 = byte +0x8] = DAT_00a7fdf0;
    }
    break;
  case 1:
  case 2:
    puVar1 = (undefined4 *)(param_1 + 0xf4 + (uint)param_2 * 0xb4);
    *puVar1 = DAT_00a7fe24;
    puVar1[1 = byte +0x4] = DAT_00a7fe28;
    puVar1[2 = byte +0x8] = DAT_00a7fe2c;
    if ((*(uint *)(param_1 + 0x110 + (uint)param_2 * 0xb4) & 1) == 0) {
      puVar1 = (undefined4 *)(param_1 + 0xf4 + (uint)param_2 * 0xb4);
      *puVar1 = DAT_00a7fe24;
      puVar1[1 = byte +0x4] = DAT_00a7fe28;
      puVar1[2 = byte +0x8] = DAT_00a7fe2c;
    }
    else {
      puVar1 = (undefined4 *)(param_1 + 0xf4 + (uint)param_2 * 0xb4);
      *puVar1 = DAT_00a7fe24;
      puVar1[1 = byte +0x4] = DAT_00a7fe28;
      puVar1[2 = byte +0x8] = DAT_00a7fe2c;
/* ... 59 more lines truncated by --max-lines */
```

---
## CARD C063
**CLAIM:** Function at 0x0089B600 is `CSWGuiLevelUpIconGrid::BuildFeatRows`: builds feat icon grid from sorted feat list, rows via 0x89BED0 (call 0x89BE91).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0089B600, size 2244, header ret=8 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x0089B600 (FUN_0089b600)
string literals (7):
  0x00995148  at 0089b886  'handmaiden'
  0x00995164  at 0089b8f7  'baodur'
  0x00995188  at 0089b98c  'kreia'
  0x0099511C  at 0089b9fa  'hanharr'
  0x0099516C  at 0089bb5f  'atton'
  0x00995164  at 0089bc68  'baodur'
  0x00995188  at 0089bd27  'kreia'
direct callees (15):
  0x0089BED0  (unnamed)
  0x006F2690  (unnamed)
  0x006F2710  (unnamed)
  0x0089B100  (unnamed)
  0x006F2610  (unnamed)
  0x006F2830  (unnamed)
  0x006A20F0  (unnamed)
  0x0084A1D0  (unnamed)
  0x00734F60  CExoString::CompareNoCase(char*)
  0x00733780  CExoString::~CExoString
  0x00844E40  CSWSCreatureStats::GetClassByPosition
  0x005933B0  (unnamed)
  0x00847A30  (unnamed)
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x0089A6E0  (unnamed)
struct offsets touched (top 11 of [REG + 0xNN], EBP/ESP excluded):
  +0x49 [dec 73]  x3  via EAX,ECX
  +0x38 [dec 56]  x2  via ECX
  +0x4E [dec 78]  x2  via ECX,EDX
  +0x32 [dec 50]  x1  via EAX
  +0x3A [dec 58]  x1  via EAX
  +0x48 [dec 72]  x1  via ECX
  +0x4A [dec 74]  x1  via ECX
  +0x4B [dec 75]  x1  via EAX

DECOMPILE (offsets annotated; truncated):
// FN 0089b600 FUN_0089b600 size=2244 ret=8 callers=1 callees=15

void FUN_0089b600(int param_1,int param_2)

{
  byte bVar1;
  uint uVar2;
  int iVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined1 local_b8 [11];
  char local_ad;
  undefined1 local_ac [11];
  char local_a1;
  undefined1 local_a0 [8];
  int local_98;
  undefined1 local_94 [8];
  int local_8c;
  undefined1 local_88 [8];
  int local_80;
  undefined1 local_7c [8];
  int local_74;
  undefined1 local_70 [8];
  int local_68;
  undefined1 local_64 [8];
  int local_5c;
  undefined1 local_58 [8];
  int local_50;
  int local_4c;
  int local_48;
  int local_44;
  undefined1 local_3d;
  int local_3c;
  uint local_38;
  int local_34;
  uint local_30;
  int local_2c;
  int local_28;
  int local_24;
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
/* ... 183 more lines truncated by --max-lines */
```

---
## CARD C064
**CLAIM:** Class `CSWGuiLevelUpPanel` has vftable 0x009AA1AC; its constructor is 0x00902F60.

```
ASM lines of ctor 0x00902F60 containing a 0x9AAxxx constant:
00902faa  c701aca19a00          MOV dword ptr [ECX],0x9aa1ac
009030d8  6894a19a00            PUSH 0x9aa194
0090311b  688ca19a00            PUSH 0x9aa18c
0090317b  687ca19a00            PUSH 0x9aa17c
009031bd  6870a19a00            PUSH 0x9aa170

ANCHORS of ctor:
anchors for 0x00902F60 (FUN_00902f60)
string literals (5):
  0x009AA194  at 009030d8  'LEVELUPPNL_p'
  0x009AA18C  at 0090311b  'LBL_%d'
  0x009AA17C  at 0090317b  'BTN_STEPNAME%d'
  0x009AA170  at 009031bd  'LBL_NUM%d'
  0x0099E67C  at 00903239  'BTN_CANCEL'
direct callees (26):
  0x0041A510  (unnamed)
  0x00847E50  CSWCLevelUpStats::CalcNumberFeats
  0x004149F0  (unnamed)
  0x0091D67E  (unnamed)

VTABLE DUMP:
0x9AA1AC (28 slots)
  slot 0 +0x0 = 0x903AA0
  slot 15 +0x3C = 0x903ED0
  slot 18 +0x48 = 0x903BA0
```

---
## CARD C065
**CLAIM:** Function at 0x00908920 is `CSWGuiFeatsCharGen::OnInputEvent`: vtable slot 15 event switch: 0x27/0x2D accept, 0x28/0x2E back, 0x29 click, 0x2A recommended, arrows.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x00908920, size 281, header ret=8 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x00908920 (FUN_00908920)
string literals (0):
direct callees (9):
  0x0040EAA0  CSWGuiPanel::HandleInputEvent
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x0089C350  (unnamed)
  0x00908A80  (unnamed)
  0x00908AE0  (unnamed)
  0x00908CC0  (unnamed)
  0x00908B50  CSWGuiPowersLevelUp::OnAcceptButton
  0x00908E30  (unnamed)
  0x00908F70  (unnamed)
struct offsets touched (top 4 of [REG + 0xNN], EBP/ESP excluded):
  +0x1C [dec 28]  x4  via EAX,ECX,EDX
  +0x3C [dec 60]  x2  via EAX,EDX
  +0xEE8 [dec 3816]  x2  via EAX,EDX
  +0x908A5C [dec 9472604]  x1  via EDX

DECOMPILE (offsets annotated; truncated):
// FN 00908920 FUN_00908920 size=281 ret=8 callers=0 callees=9

void __thiscall FUN_00908920(int param_1,undefined4 param_2,int param_3)

{
  undefined4 uVar1;
  
  if (param_3 != 0) {
    switch(param_2) {
    case 0x27:
    case 0x2d:
      FUN_004122a0(0);
      FUN_00908b50();
      break;
    case 0x28:
    case 0x2e:
      FUN_004122a0(0);
      FUN_00908ae0();
      break;
    case 0x29:
      uVar1 = FUN_00908a80();
      FUN_00908e30(uVar1);
      FUN_004122a0(0);
      break;
    case 0x2a:
      FUN_004122a0(0);
      FUN_00908cc0();
      break;
    case 0x2f:
    case 0x30:
    case 0x31:
    case 0x32:
    case 0x3d:
    case 0x3e:
    case 0x3f:
    case 0x40:
      uVar1 = FUN_0089c350(param_2);
      FUN_00908f70(uVar1);
      break;
    case 0x39:
      (**(code **)(*(int *)(param_1 + 0xee8) + 0x3c))(0x31,1);
      break;
    case 0x3a:
      (**(code **)(*(int *)(param_1 + 0xee8) + 0x3c))(0x32,1);
    }
  }
  FUN_0040eaa0(param_2,param_3);
  return;
}
```

---
## CARD C066
**CLAIM:** Function at 0x00908F70 is `CSWGuiPowersLevelUp::RefreshGridStates`: repaint power grid slot states (spells 6 and 0xE forced to 3 when panel+0x1BAC bit0 == 0).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x00908F70, size 597, header ret=4 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x00908F70 (FUN_00908f70)
string literals (0):
direct callees (7):
  0x00760EA0  (unnamed)
  0x009092D0  (unnamed)
  0x006C1450  (unnamed)
  0x00416EA0  (unnamed)
  0x00909B80  (unnamed)
  0x00416E30  CSWGuiTextParams::SetText
  0x00417010  (unnamed)
struct offsets touched (top 7 of [REG + 0xNN], EBP/ESP excluded):
  +0x4 [dec 4]  x5  via EAX,ECX,EDX
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0xC [dec 12]  x1  via EAX
  +0x10 [dec 16]  x1  via EAX
  +0x104 [dec 260]  x1  via EAX
  +0x1BB0 [dec 7088]  x1  via EDX
  +0x1C08 [dec 7176]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 00908f70 FUN_00908f70 size=597 ret=4 callers=3 callees=7

void __thiscall FUN_00908f70(int param_1,undefined4 param_2)

{
  int iVar1;
  
  FUN_009092d0(param_2,param_1 + 0x1c08);
  *(undefined4 *)(param_1 + 0x1bb0) = param_2;
  iVar1 = FUN_006c1450(param_2);
  switch(*(undefined1 *)(param_1 + 0x1c08)) {
  case 0:
    FUN_00416e30(param_1 + 0x1bb8);
    FUN_00417010(&DAT_00a7fe24);
    *(undefined4 *)(param_1 + 0x1440) = DAT_00a7fde8;
    *(undefined4 *)(param_1 + 0x1444) = DAT_00a7fdec;
    *(undefined4 *)(param_1 + 0x1448) = DAT_00a7fdf0;
    FUN_00760ea0(param_1 + 0x13a8,0x78);
    break;
  case 1:
    FUN_00416e30(param_1 + 0x1bc0);
    FUN_00417010(&DAT_00a7fe24);
    *(undefined4 *)(param_1 + 0x1440) = DAT_00a7fde8;
    *(undefined4 *)(param_1 + 0x1444) = DAT_00a7fdec;
    *(undefined4 *)(param_1 + 0x1448) = DAT_00a7fdf0;
    FUN_00760ea0(param_1 + 0x13a8,0x78);
    break;
  case 2:
    FUN_00416e30(param_1 + 0x1bc0);
    FUN_00417010(&DAT_00a7fe24);
    *(undefined4 *)(param_1 + 0x1440) = DAT_00a7fd70;
    *(undefined4 *)(param_1 + 0x1444) = DAT_00a7fd74;
    *(undefined4 *)(param_1 + 0x1448) = DAT_00a7fd78;
    break;
  case 3:
    FUN_00416e30(param_1 + 0x1bb8);
    FUN_00417010(&DAT_00a7fde8);
    *(undefined4 *)(param_1 + 0x1440) = DAT_00a7fd70;
    *(undefined4 *)(param_1 + 0x1444) = DAT_00a7fd74;
    *(undefined4 *)(param_1 + 0x1448) = DAT_00a7fd78;
    break;
  case 4:
    FUN_00416e30(param_1 + 0x1bc0);
    FUN_00417010(&DAT_00a7fe24);
    *(undefined4 *)(param_1 + 0x1440) = DAT_00a7fd70;
    *(undefined4 *)(param_1 + 0x1444) = DAT_00a7fd74;
    *(undefined4 *)(param_1 + 0x1448) = DAT_00a7fd78;
  }
  FUN_00416ea0(*(undefined4 *)(iVar1 + 0xc));
/* ... 3 more lines truncated by --max-lines */
```


# BATCH 4 (cards C067-C088)

---
## CARD C067
**CLAIM:** Function at 0x008468C0 is `CSWCLevelUpStats::DecrementSkillRank`: skill rank-down; refunds 1 (class skill) or 2 (cross-class) points.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x008468C0, size 437, header ret=8 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x008468C0 (FUN_008468c0)
string literals (0):
direct callees (5):
  0x00844E40  CSWSCreatureStats::GetClassByPosition
  0x006F24D0  (unnamed)
  0x00845C10  (unnamed)
  0x00844FF0  (unnamed)
  0x006F2440  (unnamed)
struct offsets touched (top 7 of [REG + 0xNN], EBP/ESP excluded):
  +0xB6 [dec 182]  x4  via EAX,ECX
  +0x90 [dec 144]  x2  via ECX,EDX
  +0x123 [dec 291]  x2  via EAX,EDX
  +0x1C [dec 28]  x1  via EDX
  +0x32 [dec 50]  x1  via EDX
  +0x124 [dec 292]  x1  via ECX
  +0x12C [dec 300]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 008468c0 FUN_008468c0 size=437 ret=8 callers=1 callees=5

void __thiscall FUN_008468c0(int param_1,ushort param_2,byte param_3)

{
  bool bVar1;
  byte bVar2;
  int iVar3;
  bool bVar4;
  char local_1e;
  char local_1c;
  uint local_c;
  
  bVar1 = false;
  iVar3 = (uint)param_2 * 0x20 + *(int *)(DAT_00a1b4d0 + 0x12c [dec 300]);
  if (iVar3 != 0) {
    bVar4 = (*(uint *)(iVar3 + 0x1c) >> 3 & 1) != 0;
    for (local_c = 0; (int)local_c < (int)(uint)*(byte *)(param_1 + 0x32); local_c = local_c + 1) {
      bVar2 = FUN_00844e40(local_c & 0xff);
      if (((uint)bVar2 * 0x238 + *(int *)(DAT_00a1b4d0 + 0x124) != 0) &&
         (iVar3 = FUN_006f24d0(param_2), iVar3 != 0)) {
        bVar4 = true;
        iVar3 = FUN_006f2440(param_2);
        if ((iVar3 != 0) || (iVar3 = FUN_00844ff0(param_2), iVar3 != 0)) {
          bVar1 = true;
          break;
        }
      }
    }
    if (bVar4) {
      if (param_2 < *(byte *)(DAT_00a1b4d0 + 0x123)) {
        local_1c = *(char *)((uint)param_2 + *(int *)(param_1 + 0x90));
      }
      else {
        local_1c = '\0';
      }
      if ((int)local_1c != (uint)param_3) {
        if (param_2 < *(byte *)(DAT_00a1b4d0 + 0x123)) {
          local_1e = *(char *)((uint)param_2 + *(int *)(param_1 + 0x90));
        }
        else {
          local_1e = '\0';
        }
        FUN_00845c10(param_2,local_1e + -1);
        if (bVar1) {
          *(short *)(param_1 + 0xb6) = *(short *)(param_1 + 0xb6) + 1;
        }
        else {
          *(short *)(param_1 + 0xb6) = *(short *)(param_1 + 0xb6) + 2;
/* ... 6 more lines truncated by --max-lines */
```

---
## CARD C068
**CLAIM:** Function at 0x009095F0 is `CSWGuiPowersLevelUp::BuildKnownPowerLists`: builds known-power vector (+0x1BC8) alphabetically, granted powers, picks left, then choosable 0x9098D0.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x009095F0, size 432, header ret=4 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x009095F0 (FUN_009095f0)
string literals (0):
direct callees (9):
  0x009098D0  (unnamed)
  0x00733C40  (unnamed)
  0x00733780  CExoString::~CExoString
  0x006C1450  (unnamed)
  0x004142E0  (unnamed)
  0x0049B840  (unnamed)
  0x006C12D0  (unnamed)
  0x0083EA60  CExoArrayList<uint32>::Add
  0x00414280  (unnamed)
struct offsets touched (top 4 of [REG + 0xNN], EBP/ESP excluded):
  +0x104 [dec 260]  x2  via ECX,EDX
  +0x1BA8 [dec 7080]  x2  via EAX,ECX
  +0x1BE0 [dec 7136]  x1  via EAX
  +0x1BE4 [dec 7140]  x1  via EAX

DECOMPILE (offsets annotated; truncated):
// FN 009095f0 FUN_009095f0 size=432 ret=4 callers=2 callees=9

void FUN_009095f0(undefined4 param_1)

{
  uint uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  int extraout_var;
  int iVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined1 local_2c [8];
  int local_24;
  int local_20;
  int local_1c;
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00980b80;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_20 = FUN_004142e0(param_1);
  if (local_20 != -1) {
    iVar5 = extraout_var;
    FUN_0049b840(local_20);
    local_1c = 0;
    FUN_006c1450(param_1);
    FUN_006c12d0(local_18);
    local_8 = 0;
    local_20 = 0;
    while ((local_1c == 0 && (local_20 < *(int *)(iVar5 + 0x1be4)))) {
      uVar6 = *(undefined4 *)(*(int *)(iVar5 + 0x1be0) + local_20 * 4);
      uVar7 = DAT_00a1b4d0;
      uVar2 = FUN_006c1450(uVar6);
      uVar3 = FUN_006c12d0(local_2c);
      local_8._0_1_ = 1;
      uVar4 = uVar3;
      local_24 = FUN_00733c40(local_18);
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_00733780(uVar1,uVar3,uVar4,iVar5,uVar2,uVar6,uVar7);
      if (local_24 == 0) {
        local_20 = local_20 + 1;
      }
/* ... 18 more lines truncated by --max-lines */
```

---
## CARD C069
**CLAIM:** Function at 0x00846F00 is `CSWCLevelUpStats::GetSkillPointCost`: returns 1 for class skill, 2 cross-class (2 - classSkill).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x00846F00, size 267, header ret=8 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x00846F00 (FUN_00846f00)
string literals (0):
direct callees (1):
  0x00848C30  (unnamed)
struct offsets touched (top 2 of [REG + 0xNN], EBP/ESP excluded):
  +0x38 [dec 56]  x1  via EAX
  +0x3A [dec 58]  x1  via EDX

DECOMPILE (offsets annotated; truncated):
// FN 00846f00 FUN_00846f00 size=267 ret=8 callers=2 callees=1

undefined4 FUN_00846f00(int param_1,undefined4 param_2)

{
  bool bVar1;
  bool bVar2;
  int iVar3;
  undefined4 uVar4;
  uint local_14;
  short local_c;
  
  if ((*(short *)(param_1 + 0x38) != -1) &&
     (iVar3 = FUN_00848c30(*(short *)(param_1 + 0x38),param_2), iVar3 == 0)) {
    return 0;
  }
  if ((*(short *)(param_1 + 0x3a) != -1) &&
     (iVar3 = FUN_00848c30(*(short *)(param_1 + 0x3a),param_2), iVar3 == 0)) {
    return 0;
  }
  bVar2 = false;
  bVar1 = false;
  for (local_14 = 0; (!bVar1 && ((int)local_14 < 5)); local_14 = local_14 + 1) {
    if ((byte)local_14 < 5) {
      local_c = *(short *)(param_1 + 0x3c + (local_14 & 0xff) * 2);
    }
    else {
      local_c = -1;
    }
    if (local_c != -1) {
      bVar2 = true;
      iVar3 = FUN_00848c30(local_c,param_2);
      if (iVar3 != 0) {
        bVar1 = true;
      }
    }
  }
  if ((!bVar2) || (bVar1)) {
    uVar4 = 1;
  }
  else {
    uVar4 = 0;
  }
  return uVar4;
}
```

---
## CARD C070
**CLAIM:** Function at 0x0089AAA0 is `CSWGuiLevelUpIconGrid::BuildPowerRows`: builds power icon grid from sorted spell list, rows via 0x89AF30 (0x89ADD4, 0x89AEB3).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0089AAA0, size 1095, header ret=8 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x0089AAA0 (FUN_0089aaa0)
string literals (0):
direct callees (10):
  0x0089AF30  (unnamed)
  0x006C1450  (unnamed)
  0x006BD9C0  CSWSCreatureStats::HasSpell
  0x0089A760  (unnamed)
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x006C10A0  (unnamed)
  0x0089AEF0  CSWSCreatureStats::GetClassByPosition
  0x0084A640  (unnamed)
  0x006C11C0  (unnamed)
  0x0089A6E0  (unnamed)
struct offsets touched (top 4 of [REG + 0xNN], EBP/ESP excluded):
  +0x104 [dec 260]  x2  via ECX
  +0x24 [dec 36]  x1  via ECX
  +0x8D [dec 141]  x1  via EDX
  +0x1200 [dec 4608]  x1  via EAX

DECOMPILE (offsets annotated; truncated):
// FN 0089aaa0 FUN_0089aaa0 size=1095 ret=8 callers=2 callees=10

void FUN_0089aaa0(int param_1,int param_2)

{
  bool bVar1;
  undefined1 uVar2;
  char cVar3;
  uint uVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  int local_30;
  int local_2c;
  int local_28;
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00976b28;
  local_10 = ExceptionList;
  uVar4 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_0084a640(0);
  local_8 = 0;
  FUN_0089a6e0(uVar4);
  FUN_0089a760(&local_2c);
  local_1c = 1;
  for (local_14 = 0; (local_1c != 0 && (local_14 < local_28)); local_14 = local_14 + 1) {
    iVar5 = FUN_006c1450(*(undefined4 *)(local_2c + local_14 * 4));
    if (iVar5 != 0) {
      uVar4 = FUN_006c11c0(0);
      iVar5 = FUN_006c11c0(0);
      if (iVar5 == (uVar4 / 10) * 10 || (int)(iVar5 + (uVar4 / 10) * -10) < 0) {
        bVar1 = true;
        if (param_2 != 0) {
          uVar2 = FUN_0089aef0(*(byte *)(param_1 + 0x8d) - 1);
          cVar3 = FUN_006c10a0(uVar2);
          if (cVar3 == -1) {
            bVar1 = false;
          }
        }
        if ((bVar1) &&
           ((param_2 != 0 ||
/* ... 59 more lines truncated by --max-lines */
```

---
## CARD C071
**CLAIM:** Function at 0x0090B9B0 is `CSWGuiFeatsCharGen::OnSlotClicked`: feats slot click: refusal popups strref 0xA4C8 (state 4) / 0xA4C7 / 0xA4C6 (state 2/3).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0090B9B0, size 491, header ret=4 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x0090B9B0 (FUN_0090b9b0)
string literals (0):
direct callees (8):
  0x00760EA0  (unnamed)
  0x00417140  (unnamed)
  0x00416EA0  (unnamed)
  0x0090BCB0  (unnamed)
  0x00416E30  CSWGuiTextParams::SetText
  0x00417010  (unnamed)
  0x0090C390  (unnamed)
  0x006A20F0  (unnamed)
struct offsets touched (top 5 of [REG + 0xNN], EBP/ESP excluded):
  +0x8 [dec 8]  x5  via EAX,EDX
  +0x4 [dec 4]  x4  via EAX
  +0xC [dec 12]  x1  via EDX
  +0x1BD8 [dec 7128]  x1  via EDX
  +0x1C04 [dec 7172]  x1  via EDX

DECOMPILE (offsets annotated; truncated):
// FN 0090b9b0 FUN_0090b9b0 size=491 ret=4 callers=3 callees=8

void __thiscall FUN_0090b9b0(int param_1,undefined2 param_2)

{
  int iVar1;
  
  FUN_0090bcb0(param_2,param_1 + 0x1c04);
  *(undefined2 *)(param_1 + 0x1bd8) = param_2;
  iVar1 = FUN_006a20f0(param_2);
  switch(*(undefined1 *)(param_1 + 0x1c04)) {
  case 0:
    FUN_00416e30(param_1 + 0x1be4);
    FUN_00417010(&DAT_00a7fe24);
    *(undefined4 *)(param_1 + 0x1200) = DAT_00a7fde8;
    *(undefined4 *)(param_1 + 0x1204) = DAT_00a7fdec;
    *(undefined4 *)(param_1 + 0x1208) = DAT_00a7fdf0;
    break;
  case 1:
    FUN_00416e30(param_1 + 0x1bec);
    FUN_00417010(&DAT_00a7fe24);
    *(undefined4 *)(param_1 + 0x1200) = DAT_00a7fde8;
    *(undefined4 *)(param_1 + 0x1204) = DAT_00a7fdec;
    *(undefined4 *)(param_1 + 0x1208) = DAT_00a7fdf0;
    break;
  case 2:
  case 3:
    FUN_00416e30(param_1 + 0x1bec);
    FUN_00417140(&DAT_00a7fe24);
    *(undefined4 *)(param_1 + 0x1200) = DAT_00a7fd70;
    *(undefined4 *)(param_1 + 0x1204) = DAT_00a7fd74;
    *(undefined4 *)(param_1 + 0x1208) = DAT_00a7fd78;
    break;
  case 4:
    FUN_00416e30(param_1 + 0x1be4);
    FUN_00417140(&DAT_00a7fde8);
    *(undefined4 *)(param_1 + 0x1200) = DAT_00a7fd70;
    *(undefined4 *)(param_1 + 0x1204) = DAT_00a7fd74;
    *(undefined4 *)(param_1 + 0x1208) = DAT_00a7fd78;
  }
  FUN_00416ea0(*(undefined4 *)(iVar1 + 8));
  FUN_0090c390(*(undefined4 *)(iVar1 + 0xc));
  FUN_00760ea0(param_1 + 0x1168,0x78);
  return;
}
```

---
## CARD C072
**CLAIM:** Function at 0x0089A160 is `CSWGuiLevelUpIconGrid::SetSlotState`: sets grid slot state byte/colour: 0 normal 1/2 known-granted 3 dimmed 4 selected 5/6 misc.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0089A160, size 999, header ret=8 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x0089A160 (FUN_0089a160)
string literals (0):
direct callees (0):
struct offsets touched (top 3 of [REG + 0xNN], EBP/ESP excluded):
  +0x4 [dec 4]  x11  via EAX,ECX,EDX
  +0x8 [dec 8]  x11  via EAX,ECX,EDX
  +0x24 [dec 36]  x4  via EAX,ECX,EDX

DECOMPILE (offsets annotated; truncated):
// FN 0089a160 FUN_0089a160 size=999 ret=8 callers=3 callees=0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0089a160(int param_1,byte param_2,undefined1 param_3)

{
  undefined4 *puVar1;
  int iVar2;
  
  *(undefined1 *)(param_1 + 0x10c + (uint)param_2 * 0xb4) = param_3;
  *(undefined4 *)(param_1 + 0x9c + (uint)param_2 * 0xb4) = 0x3f800000;
  *(undefined4 *)(param_1 + 0xf0 + (uint)param_2 * 0xb4) = 0x3f800000;
  if (param_2 != 0) {
    *(undefined4 *)(param_1 + 0x2b8 + (param_2 - 1) * 0x54) = 0x3f800000;
  }
  switch(param_3) {
  case 0:
    *(undefined4 *)(param_1 + 0x9c + (uint)param_2 * 0xb4) = _DAT_0098be34;
    if ((*(uint *)(param_1 + 0x110 + (uint)param_2 * 0xb4) & 1) == 0) {
      puVar1 = (undefined4 *)(param_1 + 0xf4 + (uint)param_2 * 0xb4);
      *puVar1 = DAT_00a7fde8;
      puVar1[1 = byte +0x4] = DAT_00a7fdec;
      puVar1[2 = byte +0x8] = DAT_00a7fdf0;
    }
    else {
      puVar1 = (undefined4 *)(param_1 + 0xf4 + (uint)param_2 * 0xb4);
      *puVar1 = DAT_00a7fde8;
      puVar1[1 = byte +0x4] = DAT_00a7fdec;
      puVar1[2 = byte +0x8] = DAT_00a7fdf0;
    }
    break;
  case 1:
  case 2:
    puVar1 = (undefined4 *)(param_1 + 0xf4 + (uint)param_2 * 0xb4);
    *puVar1 = DAT_00a7fe24;
    puVar1[1 = byte +0x4] = DAT_00a7fe28;
    puVar1[2 = byte +0x8] = DAT_00a7fe2c;
    if ((*(uint *)(param_1 + 0x110 + (uint)param_2 * 0xb4) & 1) == 0) {
      puVar1 = (undefined4 *)(param_1 + 0xf4 + (uint)param_2 * 0xb4);
      *puVar1 = DAT_00a7fe24;
      puVar1[1 = byte +0x4] = DAT_00a7fe28;
      puVar1[2 = byte +0x8] = DAT_00a7fe2c;
    }
    else {
      puVar1 = (undefined4 *)(param_1 + 0xf4 + (uint)param_2 * 0xb4);
      *puVar1 = DAT_00a7fe24;
      puVar1[1 = byte +0x4] = DAT_00a7fe28;
      puVar1[2 = byte +0x8] = DAT_00a7fe2c;
/* ... 59 more lines truncated by --max-lines */
```

---
## CARD C073
**CLAIM:** Class `CSWGuiLevelUpPanel` has vftable 0x009AA434; its constructor is 0x00902F60.

```
ASM lines of ctor 0x00902F60 containing a 0x9AAxxx constant:
00902faa  c701aca19a00          MOV dword ptr [ECX],0x9aa1ac
009030d8  6894a19a00            PUSH 0x9aa194
0090311b  688ca19a00            PUSH 0x9aa18c
0090317b  687ca19a00            PUSH 0x9aa17c
009031bd  6870a19a00            PUSH 0x9aa170

ANCHORS of ctor:
anchors for 0x00902F60 (FUN_00902f60)
string literals (5):
  0x009AA194  at 009030d8  'LEVELUPPNL_p'
  0x009AA18C  at 0090311b  'LBL_%d'
  0x009AA17C  at 0090317b  'BTN_STEPNAME%d'
  0x009AA170  at 009031bd  'LBL_NUM%d'
  0x0099E67C  at 00903239  'BTN_CANCEL'
direct callees (26):
  0x0041A510  (unnamed)
  0x00847E50  CSWCLevelUpStats::CalcNumberFeats
  0x004149F0  (unnamed)
  0x0091D67E  (unnamed)

VTABLE DUMP:
0x9AA434 (28 slots)
  slot 0 +0x0 = 0x90A970
  slot 15 +0x3C = 0x90B200
  slot 18 +0x48 = 0x90AD90
```

---
## CARD C074
**CLAIM:** Hook site at 0x0090C2BE inside the function at 0x0090C230: bytes `0f b7 55 e4 52`. At this point choosable-feat loop before the 4 vector probes: id = word [ebp-0x1C], panel [ebp-0x28], S [ebp-0x10]. Consumed-exit (skip) target 0x0090C349 = jmp back to loop head 0x90C27F; nothing pushed yet.

```
SITE WINDOW (>> = claimed address):
   0090C29D  39 45 e0           CMP dword ptr [EBP + -0x20],EAX
   0090C2A0  0f 8d c4 00 00 00  JGE 0x0090c36a
   0090C2A6  0f b7 4d e0        MOVZX ECX,word ptr [EBP + -0x20]
   0090C2AA  8b 15 d0 b4 a1 00  MOV EDX,dword ptr [0x00a1b4d0]
   0090C2B0  8b 82 18 01 00 00  MOV EAX,dword ptr [EDX + 0x118]
   0090C2B6  66 8b 0c 48        MOV CX,word ptr [EAX + ECX*0x2]
   0090C2BA  66 89 4d e4        MOV word ptr [EBP + -0x1c],CX
>> 0090C2BE  0f b7 55 e4        MOVZX EDX,word ptr [EBP + -0x1c]
   0090C2C2  52                 PUSH EDX
   0090C2C3  8b 4d d8           MOV ECX,dword ptr [EBP + -0x28]
   0090C2C6  81 c1 a8 1b 00 00  ADD ECX,0x1ba8
   0090C2CC  e8 4f 03 00 00     CALL 0x0090c620
   0090C2D1  83 f8 ff           CMP EAX,-0x1
   0090C2D4  0f 85 8b 00 00 00  JNZ 0x0090c365
   0090C2DA  0f b7 45 e4        MOVZX EAX,word ptr [EBP + -0x1c]

EXIT WINDOW:
   0090C341  0f b7 55 e4        MOVZX EDX,word ptr [EBP + -0x1c]
   0090C345  52                 PUSH EDX
   0090C346  8b 4d f0           MOV ECX,dword ptr [EBP + -0x10]
>> 0090C349  e8 62 bd f3 ff     CALL 0x008480b0
   0090C34E  85 c0              TEST EAX,EAX
   0090C350  74 13              JZ 0x0090c365
   0090C352  0f b7 45 e4        MOVZX EAX,word ptr [EBP + -0x1c]
```

---
## CARD C075
**CLAIM:** Class `CSWGuiSkillsCharGen` has vftable 0x009AA714; its constructor is 0x0090C6A0.

```
ASM lines of ctor 0x0090C6A0 containing a 0x9AAxxx constant:
0090c6ea  c70114a79a00          MOV dword ptr [ECX],0x9aa714
0090cac9  6800a79a00            PUSH 0x9aa700
0090cb42  682ca39a00            PUSH 0x9aa32c
0090cb7e  6818a39a00            PUSH 0x9aa318
0090cbbb  68fca29a00            PUSH 0x9aa2fc
0090cbf8  68e0a29a00            PUSH 0x9aa2e0

ANCHORS of ctor:
anchors for 0x0090C6A0 (FUN_0090c6a0)
string literals (46):
  0x009AA700  at 0090cac9  'SKCHRGEN_p'
  0x009A969C  at 0090cae9  'MAIN_TITLE_LBL'
  0x009AA32C  at 0090cb42  'SUB_TITLE_LBL'
  0x009AA318  at 0090cb7e  'REMAINING_BOX_LBL'
  0x009AA2FC  at 0090cbbb  'REMAINING_SELECTIONS_LBL'
  0x009AA2E0  at 0090cbf8  'SELECTIONS_REMAINING_LBL'
  0x009AA6F4  at 0090cc34  'COST_LBL'
  0x009AA6E4  at 0090cc71  'COST_POINTS_LBL'
  0x009AA6D4  at 0090ccae  'CLASSSKL_LBL'
  0x009A1810  at 0090ccea  'LB_DESC'

VTABLE DUMP:
0x9AA714 (28 slots)
  slot 0 +0x0 = 0x90E410
  slot 15 +0x3C = 0x90E850
  slot 18 +0x48 = 0x90E810
```

---
## CARD C076
**CLAIM:** Function at 0x009098D0 is `CSWGuiPowersLevelUp::BuildChoosablePowerVector`: builds panel+0x1BD4 choosable power vector, 0x848660 per spell id.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x009098D0, size 676, header ret=0 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x009098D0 (FUN_009098d0)
string literals (0):
direct callees (10):
  0x00848660  (unnamed)
  0x00733780  CExoString::~CExoString
  0x00733C40  (unnamed)
  0x006C1450  (unnamed)
  0x007337C0  CExoString::operator=
  0x004142E0  (unnamed)
  0x00733540  CExoString::Constructor
  0x006C12D0  (unnamed)
  0x0083EA60  CExoArrayList<uint32>::Add
  0x00414280  (unnamed)
struct offsets touched (top 7 of [REG + 0xNN], EBP/ESP excluded):
  +0x104 [dec 260]  x3  via ECX,EDX
  +0xC [dec 12]  x1  via EAX
  +0x68 [dec 104]  x1  via EAX
  +0x310 [dec 784]  x1  via ECX
  +0x1BA9 [dec 7081]  x1  via EAX
  +0x1BD4 [dec 7124]  x1  via EDX
  +0x1BD8 [dec 7128]  x1  via EDX

DECOMPILE (offsets annotated; truncated):
// FN 009098d0 FUN_009098d0 size=676 ret=0 callers=3 callees=10

void __fastcall FUN_009098d0(int param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  undefined1 local_40 [8];
  int local_38;
  undefined1 local_34 [8];
  int local_2c;
  int local_28;
  int local_24;
  ushort local_20;
  undefined1 local_1c [8];
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00980bb8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733540(uVar1);
  local_8 = 0;
  local_14 = *(undefined4 *)(*(int *)(param_1 + 0x68) + 0x310);
  for (local_20 = 0; (int)(uint)local_20 < **(int **)(DAT_00a1b4d0 + 0x104); local_20 = local_20 + 1
      ) {
    iVar2 = FUN_004142e0(local_20);
    if (((((iVar2 == -1) && (iVar2 = FUN_004142e0(local_20), iVar2 == -1)) &&
         (iVar2 = FUN_004142e0(local_20), iVar2 == -1)) &&
        ((iVar2 = FUN_004142e0(local_20), iVar2 == -1 &&
         (iVar2 = FUN_00848660(local_20,*(undefined1 *)(param_1 + 0x1ba9),param_1 + 0x1be0),
         iVar2 != 0)))) &&
       ((local_2c = FUN_006c1450(local_20), *(int *)(local_2c + 0x180) != -2 &&
        (*(int *)(local_2c + 0xc) != 0)))) {
      local_24 = 0;
      uVar3 = FUN_006c12d0(local_34);
      local_8._0_1_ = 1;
      FUN_007337c0(uVar3);
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_00733780();
      local_28 = 0;
      while ((local_24 == 0 && (local_28 < *(int *)(param_1 + 0x1bd8)))) {
        FUN_006c1450(*(undefined4 *)(*(int *)(param_1 + 0x1bd4) + local_28 * 4));
        uVar3 = FUN_006c12d0(local_40);
/* ... 24 more lines truncated by --max-lines */
```

---
## CARD C077
**CLAIM:** Function at 0x0089C170 is `CSWGuiLevelUpIconGrid::SelectSlotById`: finds the slot holding an id and selects it; silent no-op when absent.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0089C170, size 170, header ret=8 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x0089C170 (FUN_0089c170)
string literals (0):
direct callees (1):
  0x0089A160  (unnamed)
struct offsets touched (top 1 of [REG + 0xNN], EBP/ESP excluded):
  +0x4 [dec 4]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 0089c170 FUN_0089c170 size=170 ret=8 callers=2 callees=1

void __thiscall FUN_0089c170(int *param_1,int param_2,undefined1 param_3)

{
  uint local_18;
  uint local_c;
  int local_8;
  
  local_8 = 0;
  do {
    if (param_1[1 = byte +0x4] <= local_8) {
      return;
    }
    for (local_18 = 0; (int)local_18 < 3; local_18 = local_18 + 1) {
      if (*(int *)(*(int *)(*param_1 + local_8 * 4) + 0x108 + local_18 * 0xb4) == param_2) {
        local_c = local_18;
        goto LAB_0089c1ed;
      }
    }
    local_c = 0xffffffff;
LAB_0089c1ed:
    if (local_c != 0xffffffff) {
      FUN_0089a160(local_c & 0xff,param_3);
      return;
    }
    local_8 = local_8 + 1;
  } while( true );
}
```

---
## CARD C078
**CLAIM:** Function at 0x0090B6B0 is `CSWGuiFeatsCharGen::OnSlotClicked`: feats slot click: refusal popups strref 0xA4C8 (state 4) / 0xA4C7 / 0xA4C6 (state 2/3).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0090B6B0, size 472, header ret=0 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x0090B6B0 (FUN_0090b6b0)
string literals (0):
direct callees (13):
  0x0073F750  ClientApp::GetGuiInGame
  0x00840EF0  (unnamed)
  0x0084A640  (unnamed)
  0x00410530  CSWGuiManager::AddPanel
  0x0090C120  (unnamed)
  0x0089CB60  (unnamed)
  0x00844E40  CSWSCreatureStats::GetClassByPosition
  0x008489F0  CSWCLevelUpStats::AddDefaultPackageFeats
  0x0090BF90  (unnamed)
  0x0090BD50  (unnamed)
  0x005933B0  (unnamed)
  0x00416EA0  (unnamed)
  0x0064DDB0  Free(ptr*) delete-and-zero helper
struct offsets touched (top 9 of [REG + 0xNN], EBP/ESP excluded):
  +0x68 [dec 104]  x2  via EAX,ECX
  +0x310 [dec 784]  x2  via ECX,EDX
  +0x1BE0 [dec 7136]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via ECX
  +0x1C [dec 28]  x1  via EAX
  +0x32 [dec 50]  x1  via EAX
  +0xAC [dec 172]  x1  via EAX
  +0x1BCC [dec 7116]  x1  via ECX
  +0x1BD0 [dec 7120]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 0090b6b0 FUN_0090b6b0 size=472 ret=0 callers=1 callees=13

void FUN_0090b6b0(void)

{
  undefined1 uVar1;
  uint uVar2;
  int iVar3;
  int extraout_var;
  int iVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  int local_34[3 = byte +0xc];
  undefined4 local_28;
  int local_24;
  int local_20;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00981150;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_005933b0(0);
  local_8 = 0;
  iVar4 = extraout_var;
  FUN_0084a640(0);
  local_8 = CONCAT31(local_8._1_3_,1);
  local_14 = *(int *)(*(int *)(iVar4 + 0x68) + 0x310);
  uVar1 = FUN_00844e40(*(byte *)(local_14 + 0x32) - 1);
  FUN_008489f0(uVar1,&local_24,(*(uint *)(iVar4 + 0x1be0) >> 1 & 1) == 0,
               *(uint *)(iVar4 + 0x1be0) >> 1 & 1);
  while (*(int *)(iVar4 + 0x1bd0) != 0) {
    FUN_0090c120(**(undefined2 **)(iVar4 + 0x1bcc));
  }
  FUN_00840ef0(local_20);
  for (local_18 = 0; local_18 < local_20; local_18 = local_18 + 1) {
    FUN_0090bf90(*(undefined2 *)(local_24 + local_18 * 2));
    *(uint *)(local_34[0] + local_18 * 4) = (uint)*(ushort *)(local_24 + local_18 * 2);
  }
  uVar6 = *(undefined4 *)(DAT_00a1b4a4 + 4);
  iVar3 = FUN_0073f750();
  local_28 = *(undefined4 *)(iVar3 + 0xac);
  FUN_0089cb60(local_34,1);
  FUN_00416ea0(0xa510);
/* ... 10 more lines truncated by --max-lines */
```

---
## CARD C079
**CLAIM:** Function at 0x0089A0E0 is `CSWGuiLevelUpIconGrid::BuildSortedFeatList`: sorts feat ids by tier then id, caches at GUI state +0x103C.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0089A0E0, size 127, header ret=8 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x0089A0E0 (FUN_0089a0e0)
string literals (0):
direct callees (1):
  0x0089A160  (unnamed)
struct offsets touched (top 0 of [REG + 0xNN], EBP/ESP excluded):

DECOMPILE (offsets annotated; truncated):
// FN 0089a0e0 FUN_0089a0e0 size=127 ret=8 callers=3 callees=1

void __thiscall FUN_0089a0e0(int param_1,byte param_2,uint param_3)

{
  if ((*(uint *)(param_1 + 0x110 + (uint)param_2 * 0xb4) & 1) != param_3) {
    *(uint *)(param_1 + 0x110 + (uint)param_2 * 0xb4) =
         *(uint *)(param_1 + 0x110 + (uint)param_2 * 0xb4) & 0xfffffffe | param_3 & 1;
    FUN_0089a160(param_2,*(undefined1 *)(param_1 + 0x10c + (uint)param_2 * 0xb4));
  }
  return;
}
```

---
## CARD C080
**CLAIM:** Function at 0x00846690 is `CSWCLevelUpStats::IncrementSkillRank`: skill rank-up on the stats object: cap (level+3, halved cross-class), +1 rank via 0x845C10, deducts 1 or 2 points, returns bool.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x00846690, size 555, header ret=4 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x00846690 (FUN_00846690)
string literals (0):
direct callees (6):
  0x00844860  (unnamed)
  0x00844E40  CSWSCreatureStats::GetClassByPosition
  0x006F24D0  (unnamed)
  0x00845C10  (unnamed)
  0x00844FF0  (unnamed)
  0x006F2440  (unnamed)
struct offsets touched (top 7 of [REG + 0xNN], EBP/ESP excluded):
  +0xB6 [dec 182]  x6  via ECX,EDX
  +0x90 [dec 144]  x2  via EAX,EDX
  +0x123 [dec 291]  x2  via EAX,ECX
  +0x1C [dec 28]  x1  via EDX
  +0x32 [dec 50]  x1  via EDX
  +0x124 [dec 292]  x1  via ECX
  +0x12C [dec 300]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 00846690 FUN_00846690 size=555 ret=4 callers=2 callees=6

undefined4 __thiscall FUN_00846690(int param_1,ushort param_2)

{
  bool bVar1;
  byte bVar2;
  char cVar3;
  int iVar4;
  undefined4 uVar5;
  bool bVar6;
  char local_26;
  char local_24;
  byte local_d;
  uint local_c;
  
  bVar1 = false;
  iVar4 = (uint)param_2 * 0x20 + *(int *)(DAT_00a1b4d0 + 0x12c [dec 300]);
  if (iVar4 == 0) {
    uVar5 = 0;
  }
  else {
    bVar6 = (*(uint *)(iVar4 + 0x1c) >> 3 & 1) != 0;
    for (local_c = 0; (int)local_c < (int)(uint)*(byte *)(param_1 + 0x32); local_c = local_c + 1) {
      bVar2 = FUN_00844e40(local_c & 0xff);
      if (((uint)bVar2 * 0x238 + *(int *)(DAT_00a1b4d0 + 0x124) != 0) &&
         (iVar4 = FUN_006f24d0(param_2), iVar4 != 0)) {
        bVar6 = true;
        iVar4 = FUN_006f2440(param_2);
        if ((iVar4 != 0) || (iVar4 = FUN_00844ff0(param_2), iVar4 != 0)) {
          bVar1 = true;
          break;
        }
      }
    }
    if (bVar6) {
      if ((bVar1) || (1 < *(ushort *)(param_1 + 0xb6))) {
        if ((bVar1) && (*(short *)(param_1 + 0xb6) == 0)) {
          uVar5 = 0;
        }
        else {
          cVar3 = FUN_00844860();
          local_d = cVar3 + 3;
          if (!bVar1) {
            local_d = local_d / 2;
          }
          if (param_2 < *(byte *)(DAT_00a1b4d0 + 0x123)) {
            local_24 = *(char *)((uint)param_2 + *(int *)(param_1 + 0x90));
          }
/* ... 34 more lines truncated by --max-lines */
```

---
## CARD C081
**CLAIM:** Function at 0x0089AAA0 is `CSWGuiLevelUpIconGrid::BuildFeatRows`: builds feat icon grid from sorted feat list, rows via 0x89BED0 (call 0x89BE91).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0089AAA0, size 1095, header ret=8 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x0089AAA0 (FUN_0089aaa0)
string literals (0):
direct callees (10):
  0x0089AF30  (unnamed)
  0x006C1450  (unnamed)
  0x006BD9C0  CSWSCreatureStats::HasSpell
  0x0089A760  (unnamed)
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x006C10A0  (unnamed)
  0x0089AEF0  CSWSCreatureStats::GetClassByPosition
  0x0084A640  (unnamed)
  0x006C11C0  (unnamed)
  0x0089A6E0  (unnamed)
struct offsets touched (top 4 of [REG + 0xNN], EBP/ESP excluded):
  +0x104 [dec 260]  x2  via ECX
  +0x24 [dec 36]  x1  via ECX
  +0x8D [dec 141]  x1  via EDX
  +0x1200 [dec 4608]  x1  via EAX

DECOMPILE (offsets annotated; truncated):
// FN 0089aaa0 FUN_0089aaa0 size=1095 ret=8 callers=2 callees=10

void FUN_0089aaa0(int param_1,int param_2)

{
  bool bVar1;
  undefined1 uVar2;
  char cVar3;
  uint uVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  int local_30;
  int local_2c;
  int local_28;
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00976b28;
  local_10 = ExceptionList;
  uVar4 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_0084a640(0);
  local_8 = 0;
  FUN_0089a6e0(uVar4);
  FUN_0089a760(&local_2c);
  local_1c = 1;
  for (local_14 = 0; (local_1c != 0 && (local_14 < local_28)); local_14 = local_14 + 1) {
    iVar5 = FUN_006c1450(*(undefined4 *)(local_2c + local_14 * 4));
    if (iVar5 != 0) {
      uVar4 = FUN_006c11c0(0);
      iVar5 = FUN_006c11c0(0);
      if (iVar5 == (uVar4 / 10) * 10 || (int)(iVar5 + (uVar4 / 10) * -10) < 0) {
        bVar1 = true;
        if (param_2 != 0) {
          uVar2 = FUN_0089aef0(*(byte *)(param_1 + 0x8d) - 1);
          cVar3 = FUN_006c10a0(uVar2);
          if (cVar3 == -1) {
            bVar1 = false;
          }
        }
        if ((bVar1) &&
           ((param_2 != 0 ||
/* ... 59 more lines truncated by --max-lines */
```

---
## CARD C082
**CLAIM:** Function at 0x00903AA0 is `CSWGuiLevelUpPanel::Constructor`: level-up step panel ctor (+0x68 = live client creature).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x00903AA0, size 44, header ret=4 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x00903AA0 (FUN_00903aa0)
string literals (0):
direct callees (2):
  0x00903AD0  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0 of [REG + 0xNN], EBP/ESP excluded):

DECOMPILE (offsets annotated; truncated):
// FN 00903aa0 FUN_00903aa0 size=44 ret=4 callers=0 callees=2

undefined4 __thiscall FUN_00903aa0(undefined4 param_1,uint param_2)

{
  FUN_00903ad0();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

---
## CARD C083
**CLAIM:** Function at 0x00908540 is `CSWGuiPowersLevelUp::BuildKnownPowerLists`: builds known-power vector (+0x1BC8) alphabetically, granted powers, picks left, then choosable 0x9098D0.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x00908540, size 986, header ret=0 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x00908540 (FUN_00908540)
string literals (0):
direct callees (21):
  0x009098D0  (unnamed)
  0x00847F90  CSWCLevelUpStats::ComputeNumberKnownSpellsLeft
  0x006C1450  (unnamed)
  0x007337C0  CExoString::operator=
  0x0073F750  ClientApp::GetGuiInGame
  0x006C12D0  (unnamed)
  0x0084A640  (unnamed)
  0x00414280  (unnamed)
  0x0089CB60  (unnamed)
  0x00733780  CExoString::~CExoString
  0x00733C40  (unnamed)
  0x00847A70  CSWCLevelUpStats::AddGrantedPowers
  0x0049B840  (unnamed)
  0x00416EA0  (unnamed)
  0x00733540  CExoString::Constructor
  0x0083EA60  CExoArrayList<uint32>::Add
  0x00909360  (unnamed)
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x00845520  (unnamed)
  0x00476620  (unnamed)
  0x00845560  (unnamed)
struct offsets touched (top 11 of [REG + 0xNN], EBP/ESP excluded):
  +0x1BB4 [dec 7092]  x4  via EAX,ECX,EDX
  +0x104 [dec 260]  x2  via EAX
  +0x1BA8 [dec 7080]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via ECX
  +0x32 [dec 50]  x1  via ECX
  +0x68 [dec 104]  x1  via EAX
  +0xAC [dec 172]  x1  via EAX
  +0x310 [dec 784]  x1  via ECX
  +0x1BA9 [dec 7081]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 00908540 FUN_00908540 size=986 ret=0 callers=1 callees=21

void __fastcall FUN_00908540(int param_1)

{
  undefined1 uVar1;
  ushort uVar2;
  undefined4 uVar3;
  int iVar4;
  ushort uVar5;
  undefined1 local_58 [8];
  int local_50;
  undefined1 local_4c [8];
  undefined4 local_44;
  int local_40;
  int local_3c;
  int local_38;
  int local_30;
  int local_2c;
  byte local_25;
  undefined4 local_24;
  byte local_1d;
  undefined1 local_1c [8];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00980b20;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  local_14 = *(int *)(*(int *)(param_1 + 0x68) + 0x310);
  for (local_1d = 0; local_1d < *(byte *)(local_14 + 0x32); local_1d = local_1d + 1) {
    for (local_25 = 0; uVar5 = (ushort)local_25, uVar2 = FUN_00845560(local_1d,0), uVar5 < uVar2;
        local_25 = local_25 + 1) {
      local_24 = FUN_00845520(local_1d,0,local_25);
      local_2c = 0;
      local_30 = 0;
      FUN_006c1450(local_24);
      uVar3 = FUN_006c12d0(local_4c);
      local_8._0_1_ = 1;
      FUN_007337c0(uVar3);
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_00733780();
      while ((local_2c == 0 && (local_30 < *(int *)(param_1 + 0x1bcc)))) {
        FUN_006c1450(*(undefined4 *)(*(int *)(param_1 + 0x1bc8) + local_30 * 4));
/* ... 54 more lines truncated by --max-lines */
```

---
## CARD C084
**CLAIM:** Function at 0x00908920 is `CSWGuiPowersLevelUp::OnInputEvent`: vtable slot 15 event switch for powers panel.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x00908920, size 281, header ret=8 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x00908920 (FUN_00908920)
string literals (0):
direct callees (9):
  0x0040EAA0  CSWGuiPanel::HandleInputEvent
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x0089C350  (unnamed)
  0x00908A80  (unnamed)
  0x00908AE0  (unnamed)
  0x00908CC0  (unnamed)
  0x00908B50  CSWGuiPowersLevelUp::OnAcceptButton
  0x00908E30  (unnamed)
  0x00908F70  (unnamed)
struct offsets touched (top 4 of [REG + 0xNN], EBP/ESP excluded):
  +0x1C [dec 28]  x4  via EAX,ECX,EDX
  +0x3C [dec 60]  x2  via EAX,EDX
  +0xEE8 [dec 3816]  x2  via EAX,EDX
  +0x908A5C [dec 9472604]  x1  via EDX

DECOMPILE (offsets annotated; truncated):
// FN 00908920 FUN_00908920 size=281 ret=8 callers=0 callees=9

void __thiscall FUN_00908920(int param_1,undefined4 param_2,int param_3)

{
  undefined4 uVar1;
  
  if (param_3 != 0) {
    switch(param_2) {
    case 0x27:
    case 0x2d:
      FUN_004122a0(0);
      FUN_00908b50();
      break;
    case 0x28:
    case 0x2e:
      FUN_004122a0(0);
      FUN_00908ae0();
      break;
    case 0x29:
      uVar1 = FUN_00908a80();
      FUN_00908e30(uVar1);
      FUN_004122a0(0);
      break;
    case 0x2a:
      FUN_004122a0(0);
      FUN_00908cc0();
      break;
    case 0x2f:
    case 0x30:
    case 0x31:
    case 0x32:
    case 0x3d:
    case 0x3e:
    case 0x3f:
    case 0x40:
      uVar1 = FUN_0089c350(param_2);
      FUN_00908f70(uVar1);
      break;
    case 0x39:
      (**(code **)(*(int *)(param_1 + 0xee8) + 0x3c))(0x31,1);
      break;
    case 0x3a:
      (**(code **)(*(int *)(param_1 + 0xee8) + 0x3c))(0x32,1);
    }
  }
  FUN_0040eaa0(param_2,param_3);
  return;
}
```

---
## CARD C085
**CLAIM:** Function at 0x0089AEF0 is `CSWGuiLevelUpIconGrid::AddFeatRow`: allocs 0x324 grid row for (baseFeat, tier2, tier3); ids at row+0x108 + c*0xB4.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0089AEF0, size 60, header ret=4 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x0089AEF0 (FUN_0089aef0)
string literals (0):
direct callees (0):
struct offsets touched (top 1 of [REG + 0xNN], EBP/ESP excluded):
  +0x8D [dec 141]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 0089aef0 FUN_0089aef0 size=60 ret=4 callers=1 callees=0

undefined4 __thiscall FUN_0089aef0(int param_1,byte param_2)

{
  undefined4 uVar1;
  
  if (param_2 < *(byte *)(param_1 + 0x8d)) {
    uVar1 = CONCAT31((int3)((uint)param_2 * 0x28 >> 8),
                     *(undefined1 *)(param_1 + 0xab + (uint)param_2 * 0x28));
  }
  else {
    uVar1 = 0xff;
  }
  return uVar1;
}
```

---
## CARD C086
**CLAIM:** Function at 0x0089C070 is `CSWGuiLevelUpIconGrid::SelectSlotById`: finds the slot holding an id and selects it; silent no-op when absent.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0089C070, size 250, header ret=4 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x0089C070 (FUN_0089c070)
string literals (0):
direct callees (1):
  0x0089A0E0  (unnamed)
struct offsets touched (top 3 of [REG + 0xNN], EBP/ESP excluded):
  +0xC [dec 12]  x4  via EAX,ECX,EDX
  +0xD [dec 13]  x4  via EAX,ECX,EDX
  +0x4 [dec 4]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 0089c070 FUN_0089c070 size=250 ret=4 callers=5 callees=1

void __thiscall FUN_0089c070(int *param_1,int param_2)

{
  uint local_18;
  uint local_c;
  uint local_8;
  
  local_8 = 0;
  do {
    if (param_1[1 = byte +0x4] <= (int)local_8) {
      return;
    }
    for (local_18 = 0; (int)local_18 < 3; local_18 = local_18 + 1) {
      if (*(int *)(*(int *)(*param_1 + local_8 * 4) + 0x108 + local_18 * 0xb4) == param_2) {
        local_c = local_18;
        goto LAB_0089c0f1;
      }
    }
    local_c = 0xffffffff;
LAB_0089c0f1:
    if (local_c != 0xffffffff) {
      if ((*(byte *)(param_1 + 0x3 [dec 3] {int*: byte +0xc}) != local_c) || (*(byte *)((int)param_1 + 0xd) != local_8)) {
        FUN_0089a0e0((char)param_1[3 = byte +0xc],0);
        *(undefined1 *)(param_1 + 0x3 [dec 3] {int*: byte +0xc}) = (undefined1)local_c;
        *(undefined1 *)((int)param_1 + 0xd) = (undefined1)local_8;
      }
      FUN_0089a0e0((char)param_1[3 = byte +0xc],1);
      return;
    }
    local_8 = local_8 + 1;
  } while( true );
}
```

---
## CARD C087
**CLAIM:** Function at 0x0089B320 is `CSWGuiLevelUpIconGrid::BuildPowerRows`: builds power icon grid from sorted spell list, rows via 0x89AF30 (0x89ADD4, 0x89AEB3).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0089B320, size 736, header ret=4 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x0089B320 (FUN_0089b320)
string literals (0):
direct callees (8):
  0x0089BED0  (unnamed)
  0x005933B0  (unnamed)
  0x0089B100  (unnamed)
  0x006F2830  (unnamed)
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x0089A6E0  (unnamed)
  0x006B83F0  CSWSCreatureStats::HasFeat
  0x006A20F0  (unnamed)
struct offsets touched (top 4 of [REG + 0xNN], EBP/ESP excluded):
  +0x38 [dec 56]  x2  via EAX,EDX
  +0x8D [dec 141]  x2  via EAX,ECX
  +0x3A [dec 58]  x1  via EDX
  +0x124 [dec 292]  x1  via EDX

DECOMPILE (offsets annotated; truncated):
// FN 0089b320 FUN_0089b320 size=736 ret=4 callers=1 callees=8

void FUN_0089b320(void)

{
  bool bVar1;
  uint uVar2;
  int iVar3;
  undefined4 extraout_var;
  int local_3c;
  uint local_34;
  uint local_2c;
  int local_28;
  int local_24;
  int local_20;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00976b98;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_005933b0(0);
  local_8 = 0;
  FUN_0089a6e0(uVar2,extraout_var);
  FUN_0089b100(&local_24);
  bVar1 = true;
  for (local_28 = 0; (bVar1 && (local_28 < local_20)); local_28 = local_28 + 1) {
    local_18 = FUN_006a20f0(*(undefined2 *)(local_24 + local_28 * 2));
    if ((*(short *)(local_24 + local_28 * 2) != 0xcf) &&
       ((*(short *)(local_24 + local_28 * 2) != 0xee && (local_18 != 0)))) {
      if (*(short *)(local_18 + 0x38) == -1) {
        iVar3 = FUN_006b83f0(*(undefined2 *)(local_24 + local_28 * 2));
        if (iVar3 != 0) {
          local_34 = 0xffffffff;
          local_2c = 0xffffffff;
          local_3c = local_28;
          while ((local_3c = local_3c + 1, local_3c < local_20 &&
                 ((local_34 == 0xffffffff || (local_2c == 0xffffffff))))) {
            iVar3 = FUN_006b83f0(*(undefined2 *)(local_24 + local_3c * 2));
            if ((iVar3 != 0) &&
               (((local_14 = FUN_006a20f0(*(undefined2 *)(local_24 + local_3c * 2)), local_14 != 0
                 && (*(short *)(local_14 + 0x38) == *(short *)(local_24 + local_28 * 2))) &&
                ((iVar3 = FUN_006f2830(*(undefined2 *)(local_24 + local_3c * 2)), iVar3 != 0 ||
                 (iVar3 = FUN_006b83f0(*(undefined2 *)(local_24 + local_3c * 2)), iVar3 != 0)))))) {
/* ... 21 more lines truncated by --max-lines */
```

---
## CARD C088
**CLAIM:** Function at 0x0090E810 is `CSWGuiSkillsCharGen::OnInputEvent`: vtable slot 15 event switch for skills panel (R10 text says 0x90E860, vtable says 0x90E850).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0090E810, size 54, header ret=0 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x0090E810 (FUN_0090e810)
string literals (0):
direct callees (2):
  0x00740A20  CClientExoApp::ShowTutorialWindow
  0x0040F3C0  CSWGuiPanel::OnPanelAdded
struct offsets touched (top 1 of [REG + 0xNN], EBP/ESP excluded):
  +0x4 [dec 4]  x1  via EAX

DECOMPILE (offsets annotated; truncated):
// FN 0090e810 FUN_0090e810 size=54 ret=0 callers=0 callees=2

void __fastcall FUN_0090e810(undefined4 param_1)

{
  FUN_0040f3c0(param_1);
  FUN_00740a20(0x11,0x7f000000,0x7f000000,0);
  return;
}
```


# BATCH 5 (cards C089-C110)

---
## CARD C089
**CLAIM:** Class `CSWGuiPowersLevelUp` has vftable 0x009AA714; its constructor is 0x009074E0.

```
ASM lines of ctor 0x009074E0 containing a 0x9AAxxx constant:
0090752a  c7014ca39a00          MOV dword ptr [ECX],0x9aa34c
009077e9  683ca39a00            PUSH 0x9aa33c
00907843  682ca39a00            PUSH 0x9aa32c
00907880  6818a39a00            PUSH 0x9aa318
009078bd  68fca29a00            PUSH 0x9aa2fc
009078f9  68e0a29a00            PUSH 0x9aa2e0

ANCHORS of ctor:
anchors for 0x009074E0 (FUN_009074e0)
string literals (15):
  0x009AA33C  at 009077e9  'pwrlvlup_p'
  0x009A969C  at 00907809  'MAIN_TITLE_LBL'
  0x009AA32C  at 00907843  'SUB_TITLE_LBL'
  0x009AA318  at 00907880  'REMAINING_BOX_LBL'
  0x009AA2FC  at 009078bd  'REMAINING_SELECTIONS_LBL'
  0x009AA2E0  at 009078f9  'SELECTIONS_REMAINING_LBL'
  0x009A1810  at 00907936  'LB_DESC'
  0x009AA2D4  at 00907980  'LB_POWERS'
  0x009AA2C8  at 009079bd  'LBL_POWER'
  0x009AA2B8  at 009079f9  'RECOMMENDED_BTN'

VTABLE DUMP:
0x9AA714 (28 slots)
  slot 0 +0x0 = 0x90E410
  slot 15 +0x3C = 0x90E850
  slot 18 +0x48 = 0x90E810
```

---
## CARD C090
**CLAIM:** Function at 0x0090BD50 is `CSWGuiPowersLevelUp::RefreshGridStates`: repaint power grid slot states (spells 6 and 0xE forced to 3 when panel+0x1BAC bit0 == 0).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0090BD50, size 570, header ret=0 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x0090BD50 (FUN_0090bd50)
string literals (0):
direct callees (10):
  0x00733700  CExoString::Constructor_5
  0x00733780  CExoString::~CExoString
  0x0090B9B0  (unnamed)
  0x0041FC30  (unnamed)
  0x0089C170  (unnamed)
  0x0089C070  (unnamed)
  0x0089C220  (unnamed)
  0x0089C290  (unnamed)
  0x00416E30  CSWGuiTextParams::SetText
  0x0041E870  (unnamed)
struct offsets touched (top 11 of [REG + 0xNN], EBP/ESP excluded):
  +0x1BD8 [dec 7128]  x4  via EAX,EDX
  +0x1BA8 [dec 7080]  x1  via ECX
  +0x1BAC [dec 7084]  x1  via ECX
  +0x1BB4 [dec 7092]  x1  via ECX
  +0x1BB8 [dec 7096]  x1  via ECX
  +0x1BC0 [dec 7104]  x1  via EDX
  +0x1BC4 [dec 7108]  x1  via EDX
  +0x1BCC [dec 7116]  x1  via EAX
  +0x1BD0 [dec 7120]  x1  via EAX
  +0x1BDE [dec 7134]  x1  via ECX
  +0x1C01 [dec 7169]  x1  via EDX

DECOMPILE (offsets annotated; truncated):
// FN 0090bd50 FUN_0090bd50 size=570 ret=0 callers=3 callees=10

void __fastcall FUN_0090bd50(int param_1)

{
  undefined2 uVar1;
  undefined1 local_1c [8];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a238;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0089c220(3);
  for (local_14 = 0; local_14 < *(int *)(param_1 + 0x1bac); local_14 = local_14 + 1) {
    FUN_0089c170(*(undefined2 *)(*(int *)(param_1 + 0x1ba8) + local_14 * 2),1);
  }
  for (local_14 = 0; local_14 < *(int *)(param_1 + 0x1bd0); local_14 = local_14 + 1) {
    FUN_0089c170(*(undefined2 *)(*(int *)(param_1 + 0x1bcc) + local_14 * 2),4);
  }
  for (local_14 = 0; local_14 < *(int *)(param_1 + 0x1bc4); local_14 = local_14 + 1) {
    FUN_0089c170(*(undefined2 *)(*(int *)(param_1 + 0x1bc0) + local_14 * 2),0);
  }
  for (local_14 = 0; local_14 < *(int *)(param_1 + 0x1bb8); local_14 = local_14 + 1) {
    FUN_0089c170(*(undefined2 *)(*(int *)(param_1 + 0x1bb4) + local_14 * 2),2);
  }
  if (*(short *)(param_1 + 0x1bd8) == -1) {
    FUN_0041fc30(param_1 + 0x1bf4,0,1,0);
    uVar1 = FUN_0089c290();
    *(undefined2 *)(param_1 + 0x1bd8) = uVar1;
  }
  FUN_0089c070(*(undefined2 *)(param_1 + 0x1bd8));
  FUN_0041e870(*(undefined1 *)(param_1 + 0x1c01),0);
  FUN_0090b9b0(*(undefined2 *)(param_1 + 0x1bd8));
  FUN_00733700(*(undefined1 *)(param_1 + 0x1bde));
  local_8 = 0;
  FUN_00416e30(local_1c);
  local_8 = 0xffffffff;
  FUN_00733780();
  ExceptionList = local_10;
  return;
}
```

---
## CARD C091
**CLAIM:** Function at 0x00909360 is `CSWGuiFeatsCharGen::RefreshGridStates`: repaint feat grid slot states: 0x89C220(3) then known->1 selected->4 choosable->0 granted->2.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x00909360, size 644, header ret=0 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x00909360 (FUN_00909360)
string literals (0):
direct callees (10):
  0x00733700  CExoString::Constructor_5
  0x00733780  CExoString::~CExoString
  0x0041FC30  (unnamed)
  0x0089C170  (unnamed)
  0x0089C070  (unnamed)
  0x0089C220  (unnamed)
  0x0089C290  (unnamed)
  0x00416E30  CSWGuiTextParams::SetText
  0x0041E870  (unnamed)
  0x00908F70  (unnamed)
struct offsets touched (top 12 of [REG + 0xNN], EBP/ESP excluded):
  +0x1BB0 [dec 7088]  x4  via ECX,EDX
  +0x1BD4 [dec 7124]  x4  via ECX,EDX
  +0x1BA8 [dec 7080]  x1  via EAX
  +0x1BAC [dec 7084]  x1  via EDX
  +0x1BC8 [dec 7112]  x1  via ECX
  +0x1BCC [dec 7116]  x1  via ECX
  +0x1BD8 [dec 7128]  x1  via EDX
  +0x1BE0 [dec 7136]  x1  via EAX
  +0x1BE4 [dec 7140]  x1  via EAX
  +0x1BEC [dec 7148]  x1  via ECX
  +0x1BF0 [dec 7152]  x1  via ECX
  +0x1C05 [dec 7173]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 00909360 FUN_00909360 size=644 ret=0 callers=3 callees=10

void __fastcall FUN_00909360(int param_1)

{
  undefined4 uVar1;
  undefined1 local_1c [8];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a238;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0089c220(3);
  for (local_14 = 0; local_14 < *(int *)(param_1 + 0x1bcc); local_14 = local_14 + 1) {
    FUN_0089c170(*(undefined4 *)(*(int *)(param_1 + 0x1bc8) + local_14 * 4),1);
  }
  for (local_14 = 0; local_14 < *(int *)(param_1 + 0x1be4); local_14 = local_14 + 1) {
    FUN_0089c170(*(undefined4 *)(*(int *)(param_1 + 0x1be0) + local_14 * 4),4);
  }
  for (local_14 = 0; local_14 < *(int *)(param_1 + 0x1bd8); local_14 = local_14 + 1) {
    if (((*(uint *)(param_1 + 0x1bac) & 1) == 0) &&
       ((*(int *)(*(int *)(param_1 + 0x1bd4) + local_14 * 4) == 6 ||
        (*(int *)(*(int *)(param_1 + 0x1bd4) + local_14 * 4) == 0xe)))) {
      FUN_0089c170(*(undefined4 *)(*(int *)(param_1 + 0x1bd4) + local_14 * 4),3);
    }
    else {
      FUN_0089c170(*(undefined4 *)(*(int *)(param_1 + 0x1bd4) + local_14 * 4),0);
    }
  }
  for (local_14 = 0; local_14 < *(int *)(param_1 + 0x1bf0); local_14 = local_14 + 1) {
    FUN_0089c170(*(undefined4 *)(*(int *)(param_1 + 0x1bec) + local_14 * 4),2);
  }
  if (*(int *)(param_1 + 0x1bb0) == -1) {
    FUN_0041fc30(param_1 + 0x1bf8,0,1,0);
    uVar1 = FUN_0089c290();
    *(undefined4 *)(param_1 + 0x1bb0) = uVar1;
  }
  FUN_0089c070(*(undefined4 *)(param_1 + 0x1bb0));
  FUN_0041e870(*(undefined1 *)(param_1 + 0x1c05),0);
  FUN_00908f70(*(undefined4 *)(param_1 + 0x1bb0));
  FUN_00733700(*(undefined1 *)(param_1 + 0x1ba8));
  local_8 = 0;
  FUN_00416e30(local_1c);
  local_8 = 0xffffffff;
  FUN_00733780();
/* ... 3 more lines truncated by --max-lines */
```

---
## CARD C092
**CLAIM:** Function at 0x00908040 is `CSWGuiPowersLevelUp::Constructor`: powers level-up panel ctor (grid ptr at +0x1BF8).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x00908040, size 44, header ret=4 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x00908040 (FUN_00908040)
string literals (0):
direct callees (2):
  0x00908070  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0 of [REG + 0xNN], EBP/ESP excluded):

DECOMPILE (offsets annotated; truncated):
// FN 00908040 FUN_00908040 size=44 ret=4 callers=0 callees=2

undefined4 __thiscall FUN_00908040(undefined4 param_1,uint param_2)

{
  FUN_00908070();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

---
## CARD C093
**CLAIM:** Function at 0x0089A760 is `CSWGuiLevelUpIconGrid::BuildSortedPowerList`: sorts spells.2da rows (type +0x14C in {1,6}, name strref != 0) by multi-digit key, caches at +0x1048.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0089A760, size 829, header ret=4 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x0089A760 (FUN_0089a760)
string literals (0):
direct callees (6):
  0x006C1450  (unnamed)
  0x0073F750  ClientApp::GetGuiInGame
  0x0040E120  (unnamed)
  0x0083EA60  CExoArrayList<uint32>::Add
  0x006C11C0  (unnamed)
  0x00414280  (unnamed)
struct offsets touched (top 4 of [REG + 0xNN], EBP/ESP excluded):
  +0x4 [dec 4]  x4  via EAX,EDX
  +0x104 [dec 260]  x3  via ECX,EDX
  +0x14C [dec 332]  x2  via EAX
  +0xC [dec 12]  x1  via EAX

DECOMPILE (offsets annotated; truncated):
// FN 0089a760 FUN_0089a760 size=829 ret=4 callers=1 callees=6

void FUN_0089a760(int *param_1)

{
  bool bVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  int iVar5;
  int local_14;
  int local_c;
  
  iVar2 = FUN_0073f750();
  if ((iVar2 + 0x1048 == 0) || (*(int *)(iVar2 + 0x104c) < 1)) {
    local_c = 0;
    while( true ) {
      if (**(int **)(DAT_00a1b4d0 + 0x104) <= local_c) break;
      iVar2 = FUN_006c1450(local_c);
      if ((iVar2 != 0) &&
         (((*(char *)(iVar2 + 0x14c) == '\x01' || (*(char *)(iVar2 + 0x14c) == '\x06')) &&
          (*(int *)(iVar2 + 0xc) != 0)))) {
        bVar1 = false;
        for (local_14 = 0; local_14 < param_1[1 = byte +0x4]; local_14 = local_14 + 1) {
          iVar2 = FUN_006c1450(*(undefined4 *)(*param_1 + local_14 * 4));
          if (iVar2 != 0) {
            iVar2 = FUN_006c11c0(0);
            uVar3 = FUN_006c11c0(0);
            uVar4 = FUN_006c11c0(0);
            iVar5 = FUN_006c11c0(0);
            if ((int)(iVar5 + (uVar4 / 10) * -10) < (int)(iVar2 + (uVar3 / 10) * -10)) {
              bVar1 = true;
            }
            else {
              iVar2 = FUN_006c11c0(0);
              uVar3 = FUN_006c11c0(0);
              uVar4 = FUN_006c11c0(0);
              iVar5 = FUN_006c11c0(0);
              if (iVar2 + (uVar3 / 10) * -10 == iVar5 + (uVar4 / 10) * -10) {
                iVar2 = FUN_006c11c0(0);
                iVar5 = FUN_006c11c0(0);
                if (iVar2 / 1000 < iVar5 / 1000) {
                  bVar1 = true;
                }
                else {
                  iVar2 = FUN_006c11c0(0);
                  iVar5 = FUN_006c11c0(0);
                  if (iVar2 / 1000 == iVar5 / 1000) {
                    iVar2 = FUN_006c11c0(0);
/* ... 31 more lines truncated by --max-lines */
```

---
## CARD C094
**CLAIM:** Function at 0x0090FBD0 is `CSWGuiSkillsCharGen::RefreshRowStyling`: skills row dimming: disabled = !(skillRow+0x1C bit4), colour DAT_00A7FD4C vs DAT_00A7FDAC.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0090FBD0, size 1727, header ret=0 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x0090FBD0 (FUN_0090fbd0)
string literals (0):
direct callees (14):
  0x00845C10  (unnamed)
  0x00416E30  CSWGuiTextParams::SetText
  0x008444B0  (unnamed)
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x00734F60  CExoString::CompareNoCase(char*)
  0x00733700  CExoString::Constructor_5
  0x00733780  CExoString::~CExoString
  0x00844E40  CSWSCreatureStats::GetClassByPosition
  0x00844860  (unnamed)
  0x00417010  (unnamed)
  0x00844620  (unnamed)
  0x00846200  (unnamed)
  0x00848870  CSWCLevelUpStats::AddDefaultPackageSkills
  0x009103A0  (unnamed)
struct offsets touched (top 12 of [REG + 0xNN], EBP/ESP excluded):
  +0x68 [dec 104]  x17  via EAX,ECX,EDX
  +0x310 [dec 784]  x16  via EAX,ECX,EDX
  +0x48 [dec 72]  x8  via EAX,ECX,EDX
  +0x123 [dec 291]  x6  via ECX,EDX
  +0x4CF8 [dec 19704]  x5  via EAX,ECX
  +0x90 [dec 144]  x4  via EAX,ECX
  +0xB6 [dec 182]  x3  via EAX,ECX
  +0x1C [dec 28]  x2  via ECX,EDX
  +0x4CE8 [dec 19688]  x2  via EAX,ECX
  +0x32 [dec 50]  x1  via EAX
  +0x12C [dec 300]  x1  via ECX
  +0x4CFC [dec 19708]  x1  via EDX

DECOMPILE (offsets annotated; truncated):
// FN 0090fbd0 FUN_0090fbd0 size=1727 ret=0 callers=1 callees=14

void __fastcall FUN_0090fbd0(int param_1)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  char local_a1;
  char local_91;
  char local_7d;
  char local_71;
  undefined1 local_34 [8];
  undefined1 local_2c [8];
  int local_24;
  undefined1 local_1e;
  char local_1d;
  int local_1c;
  int local_18;
  byte local_11;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009818b0;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_1d = *(char *)(*(int *)(*(int *)(param_1 + 0x68) + 0x310) + 0x32) + -1;
  local_1e = FUN_00844e40(local_1d);
  if ((*(uint *)(param_1 + 0x4cf8) & 1) == 0) {
    *(undefined2 *)(*(int *)(*(int *)(param_1 + 0x68) + 0x310) + 0xb6) = 0;
    FUN_00846200(uVar1);
    FUN_008444b0();
    FUN_00844620();
  }
  else {
    *(undefined2 *)(*(int *)(*(int *)(param_1 + 0x68) + 0x310) + 0xb6) =
         *(undefined2 *)(param_1 + 0x4cfc);
  }
  for (local_11 = 0; local_11 < *(byte *)(DAT_00a1b4d0 + 0x123); local_11 = local_11 + 1) {
    FUN_00845c10(local_11,*(undefined1 *)(param_1 + 0x4cf0 + (uint)local_11));
  }
  FUN_00848870(local_1e,*(uint *)(param_1 + 0x4cf8) >> 1 & 1);
  for (local_11 = 0; local_11 < *(byte *)(DAT_00a1b4d0 + 0x123); local_11 = local_11 + 1) {
    uVar2 = FUN_00844860();
    local_1c = (uVar2 & 0xff) + 3;
    iVar3 = FUN_009103a0(local_11);
/* ... 81 more lines truncated by --max-lines */
```

---
## CARD C095
**CLAIM:** Call site at 0x009099DB in the function at 0x009098D0: bytes `e8 80 ec f3 ff` = call to 0x00848660 [unnamed-in-DB power-availability test (candidate in this packet)]; 0x848660(S, spell, +0x1BA9, &selected +0x1BE0): 3 stack args pushed at 0x9099BE-0x9099D7.

```
CALL WINDOW (>> = claimed address):
   009099C1  81 c2 e0 1b 00 00  ADD EDX,0x1be0
   009099C7  52                 PUSH EDX
   009099C8  8b 45 98           MOV EAX,dword ptr [EBP + -0x68]
   009099CB  0f b6 88 a9 1b 00 00 MOVZX ECX,byte ptr [EAX + 0x1ba9]
   009099D2  51                 PUSH ECX
   009099D3  0f b7 55 e4        MOVZX EDX,word ptr [EBP + -0x1c]
   009099D7  52                 PUSH EDX
   009099D8  8b 4d f0           MOV ECX,dword ptr [EBP + -0x10]
>> 009099DB  e8 80 ec f3 ff     CALL 0x00848660
   009099E0  85 c0              TEST EAX,EAX
   009099E2  0f 84 69 01 00 00  JZ 0x00909b51
   009099E8  a1 d0 b4 a1 00     MOV EAX,[0x00a1b4d0]
   009099ED  89 45 b8           MOV dword ptr [EBP + -0x48],EAX
```

---
## CARD C096
**CLAIM:** Hook site at 0x0090C2C3 inside the function at 0x0090C230: bytes `0f b7 55 e4 52`. At this point choosable-feat loop before the 4 vector probes: id = word [ebp-0x1C], panel [ebp-0x28], S [ebp-0x10]. Consumed-exit (skip) target 0x0090C365 = jmp back to loop head 0x90C27F; nothing pushed yet.

```
SITE WINDOW (>> = claimed address):
   0090C2A6  0f b7 4d e0        MOVZX ECX,word ptr [EBP + -0x20]
   0090C2AA  8b 15 d0 b4 a1 00  MOV EDX,dword ptr [0x00a1b4d0]
   0090C2B0  8b 82 18 01 00 00  MOV EAX,dword ptr [EDX + 0x118]
   0090C2B6  66 8b 0c 48        MOV CX,word ptr [EAX + ECX*0x2]
   0090C2BA  66 89 4d e4        MOV word ptr [EBP + -0x1c],CX
   0090C2BE  0f b7 55 e4        MOVZX EDX,word ptr [EBP + -0x1c]
   0090C2C2  52                 PUSH EDX
>> 0090C2C3  8b 4d d8           MOV ECX,dword ptr [EBP + -0x28]
   0090C2C6  81 c1 a8 1b 00 00  ADD ECX,0x1ba8
   0090C2CC  e8 4f 03 00 00     CALL 0x0090c620
   0090C2D1  83 f8 ff           CMP EAX,-0x1
   0090C2D4  0f 85 8b 00 00 00  JNZ 0x0090c365
   0090C2DA  0f b7 45 e4        MOVZX EAX,word ptr [EBP + -0x1c]
   0090C2DE  50                 PUSH EAX
   0090C2DF  8b 4d d8           MOV ECX,dword ptr [EBP + -0x28]

EXIT WINDOW:
   0090C357  8b 4d d8           MOV ECX,dword ptr [EBP + -0x28]
   0090C35A  81 c1 c0 1b 00 00  ADD ECX,0x1bc0
   0090C360  e8 eb 70 c8 ff     CALL 0x00593450
>> 0090C365  e9 15 ff ff ff     JMP 0x0090c27f
   0090C36A  c7 45 fc ff ff ff ff MOV dword ptr [EBP + -0x4],0xffffffff
   0090C371  8d 4d e8           LEA ECX,[EBP + -0x18]
   0090C374  e8 07 74 e2 ff     CALL 0x00733780
```

---
## CARD C097
**CLAIM:** Function at 0x009098D0 is `CSWGuiFeatsCharGen::BuildChoosableFeatVector`: builds panel+0x1BC0 choosable feat vector, CanChooseFeat per id.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x009098D0, size 676, header ret=0 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x009098D0 (FUN_009098d0)
string literals (0):
direct callees (10):
  0x00848660  (unnamed)
  0x00733780  CExoString::~CExoString
  0x00733C40  (unnamed)
  0x006C1450  (unnamed)
  0x007337C0  CExoString::operator=
  0x004142E0  (unnamed)
  0x00733540  CExoString::Constructor
  0x006C12D0  (unnamed)
  0x0083EA60  CExoArrayList<uint32>::Add
  0x00414280  (unnamed)
struct offsets touched (top 7 of [REG + 0xNN], EBP/ESP excluded):
  +0x104 [dec 260]  x3  via ECX,EDX
  +0xC [dec 12]  x1  via EAX
  +0x68 [dec 104]  x1  via EAX
  +0x310 [dec 784]  x1  via ECX
  +0x1BA9 [dec 7081]  x1  via EAX
  +0x1BD4 [dec 7124]  x1  via EDX
  +0x1BD8 [dec 7128]  x1  via EDX

DECOMPILE (offsets annotated; truncated):
// FN 009098d0 FUN_009098d0 size=676 ret=0 callers=3 callees=10

void __fastcall FUN_009098d0(int param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  undefined1 local_40 [8];
  int local_38;
  undefined1 local_34 [8];
  int local_2c;
  int local_28;
  int local_24;
  ushort local_20;
  undefined1 local_1c [8];
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00980bb8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733540(uVar1);
  local_8 = 0;
  local_14 = *(undefined4 *)(*(int *)(param_1 + 0x68) + 0x310);
  for (local_20 = 0; (int)(uint)local_20 < **(int **)(DAT_00a1b4d0 + 0x104); local_20 = local_20 + 1
      ) {
    iVar2 = FUN_004142e0(local_20);
    if (((((iVar2 == -1) && (iVar2 = FUN_004142e0(local_20), iVar2 == -1)) &&
         (iVar2 = FUN_004142e0(local_20), iVar2 == -1)) &&
        ((iVar2 = FUN_004142e0(local_20), iVar2 == -1 &&
         (iVar2 = FUN_00848660(local_20,*(undefined1 *)(param_1 + 0x1ba9),param_1 + 0x1be0),
         iVar2 != 0)))) &&
       ((local_2c = FUN_006c1450(local_20), *(int *)(local_2c + 0x180) != -2 &&
        (*(int *)(local_2c + 0xc) != 0)))) {
      local_24 = 0;
      uVar3 = FUN_006c12d0(local_34);
      local_8._0_1_ = 1;
      FUN_007337c0(uVar3);
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_00733780();
      local_28 = 0;
      while ((local_24 == 0 && (local_28 < *(int *)(param_1 + 0x1bd8)))) {
        FUN_006c1450(*(undefined4 *)(*(int *)(param_1 + 0x1bd4) + local_28 * 4));
        uVar3 = FUN_006c12d0(local_40);
/* ... 24 more lines truncated by --max-lines */
```

---
## CARD C098
**CLAIM:** Call site at 0x0090C345 in the function at 0x0090C230: bytes `e8 62 bd f3 ff` = call to 0x008480B0 [CanChooseFeat (existing DB, high)]; CanChooseFeat(S, feat, +0x1BDC, +0x1BDD, &selected) with ecx=S=[ebp-0x10], 4 stack args, callee-cleaned RET 0x10; true result pushes into +0x1BC0.

```
CALL WINDOW (>> = claimed address):
   0090C32A  50                 PUSH EAX
   0090C32B  8b 4d d8           MOV ECX,dword ptr [EBP + -0x28]
   0090C32E  0f b6 91 dd 1b 00 00 MOVZX EDX,byte ptr [ECX + 0x1bdd]
   0090C335  52                 PUSH EDX
   0090C336  8b 45 d8           MOV EAX,dword ptr [EBP + -0x28]
   0090C339  0f b6 88 dc 1b 00 00 MOVZX ECX,byte ptr [EAX + 0x1bdc]
   0090C340  51                 PUSH ECX
   0090C341  0f b7 55 e4        MOVZX EDX,word ptr [EBP + -0x1c]
>> 0090C345  52                 PUSH EDX
   0090C346  8b 4d f0           MOV ECX,dword ptr [EBP + -0x10]
   0090C349  e8 62 bd f3 ff     CALL 0x008480b0
   0090C34E  85 c0              TEST EAX,EAX
   0090C350  74 13              JZ 0x0090c365
```

---
## CARD C099
**CLAIM:** Function at 0x0089C220 is `CSWGuiLevelUpIconGrid::SetSlotState`: sets grid slot state byte/colour: 0 normal 1/2 known-granted 3 dimmed 4 selected 5/6 misc.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0089C220, size 104, header ret=4 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x0089C220 (FUN_0089c220)
string literals (0):
direct callees (1):
  0x0089A160  (unnamed)
struct offsets touched (top 1 of [REG + 0xNN], EBP/ESP excluded):
  +0x4 [dec 4]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 0089c220 FUN_0089c220 size=104 ret=4 callers=4 callees=1

void __thiscall FUN_0089c220(int param_1,undefined1 param_2)

{
  undefined4 local_c;
  undefined4 local_8;
  
  for (local_8 = 0; local_8 < *(int *)(param_1 + 4); local_8 = local_8 + 1) {
    for (local_c = 0; (int)local_c < 3; local_c = local_c + 1) {
      FUN_0089a160(local_c & 0xff,param_2);
    }
  }
  return;
}
```

---
## CARD C100
**CLAIM:** Function at 0x008468C0 is `CSWCLevelUpStats::IncrementSkillRank`: skill rank-up on the stats object: cap (level+3, halved cross-class), +1 rank via 0x845C10, deducts 1 or 2 points, returns bool.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x008468C0, size 437, header ret=8 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x008468C0 (FUN_008468c0)
string literals (0):
direct callees (5):
  0x00844E40  CSWSCreatureStats::GetClassByPosition
  0x006F24D0  (unnamed)
  0x00845C10  (unnamed)
  0x00844FF0  (unnamed)
  0x006F2440  (unnamed)
struct offsets touched (top 7 of [REG + 0xNN], EBP/ESP excluded):
  +0xB6 [dec 182]  x4  via EAX,ECX
  +0x90 [dec 144]  x2  via ECX,EDX
  +0x123 [dec 291]  x2  via EAX,EDX
  +0x1C [dec 28]  x1  via EDX
  +0x32 [dec 50]  x1  via EDX
  +0x124 [dec 292]  x1  via ECX
  +0x12C [dec 300]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 008468c0 FUN_008468c0 size=437 ret=8 callers=1 callees=5

void __thiscall FUN_008468c0(int param_1,ushort param_2,byte param_3)

{
  bool bVar1;
  byte bVar2;
  int iVar3;
  bool bVar4;
  char local_1e;
  char local_1c;
  uint local_c;
  
  bVar1 = false;
  iVar3 = (uint)param_2 * 0x20 + *(int *)(DAT_00a1b4d0 + 0x12c [dec 300]);
  if (iVar3 != 0) {
    bVar4 = (*(uint *)(iVar3 + 0x1c) >> 3 & 1) != 0;
    for (local_c = 0; (int)local_c < (int)(uint)*(byte *)(param_1 + 0x32); local_c = local_c + 1) {
      bVar2 = FUN_00844e40(local_c & 0xff);
      if (((uint)bVar2 * 0x238 + *(int *)(DAT_00a1b4d0 + 0x124) != 0) &&
         (iVar3 = FUN_006f24d0(param_2), iVar3 != 0)) {
        bVar4 = true;
        iVar3 = FUN_006f2440(param_2);
        if ((iVar3 != 0) || (iVar3 = FUN_00844ff0(param_2), iVar3 != 0)) {
          bVar1 = true;
          break;
        }
      }
    }
    if (bVar4) {
      if (param_2 < *(byte *)(DAT_00a1b4d0 + 0x123)) {
        local_1c = *(char *)((uint)param_2 + *(int *)(param_1 + 0x90));
      }
      else {
        local_1c = '\0';
      }
      if ((int)local_1c != (uint)param_3) {
        if (param_2 < *(byte *)(DAT_00a1b4d0 + 0x123)) {
          local_1e = *(char *)((uint)param_2 + *(int *)(param_1 + 0x90));
        }
        else {
          local_1e = '\0';
        }
        FUN_00845c10(param_2,local_1e + -1);
        if (bVar1) {
          *(short *)(param_1 + 0xb6) = *(short *)(param_1 + 0xb6) + 1;
        }
        else {
          *(short *)(param_1 + 0xb6) = *(short *)(param_1 + 0xb6) + 2;
/* ... 6 more lines truncated by --max-lines */
```

---
## CARD C101
**CLAIM:** Function at 0x009092D0 is `CSWGuiPowersLevelUp::BuildChoosablePowerVector`: builds panel+0x1BD4 choosable power vector, 0x848660 per spell id.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x009092D0, size 143, header ret=8 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x009092D0 (FUN_009092d0)
string literals (0):
direct callees (1):
  0x004142E0  (unnamed)
struct offsets touched (top 0 of [REG + 0xNN], EBP/ESP excluded):

DECOMPILE (offsets annotated; truncated):
// FN 009092d0 FUN_009092d0 size=143 ret=8 callers=2 callees=1

void FUN_009092d0(undefined4 param_1,undefined1 *param_2)

{
  int iVar1;
  
  iVar1 = FUN_004142e0(param_1);
  if (iVar1 == -1) {
    iVar1 = FUN_004142e0(param_1);
    if (iVar1 == -1) {
      iVar1 = FUN_004142e0(param_1);
      if (iVar1 == -1) {
        iVar1 = FUN_004142e0(param_1);
        if (iVar1 == -1) {
          *param_2 = 3;
        }
        else {
          *param_2 = 4;
        }
      }
      else {
        *param_2 = 0;
      }
    }
    else {
      *param_2 = 1;
    }
  }
  else {
    *param_2 = 2;
  }
  return;
}
```

---
## CARD C102
**CLAIM:** Function at 0x0090B200 is `CSWGuiPowersLevelUp::OnInputEvent`: vtable slot 15 event switch for powers panel.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0090B200, size 281, header ret=8 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x0090B200 (FUN_0090b200)
string literals (0):
direct callees (9):
  0x0090B890  (unnamed)
  0x0040EAA0  CSWGuiPanel::HandleInputEvent
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x0089C350  (unnamed)
  0x0090B9B0  (unnamed)
  0x00908A80  (unnamed)
  0x0090B360  (unnamed)
  0x0090B430  CSWGuiFeatsCharGen::OnAcceptButton
  0x0090B6B0  (unnamed)
struct offsets touched (top 4 of [REG + 0xNN], EBP/ESP excluded):
  +0x1C [dec 28]  x4  via EAX,ECX,EDX
  +0x3C [dec 60]  x2  via EAX,EDX
  +0x18B8 [dec 6328]  x2  via EAX,EDX
  +0x90B33C [dec 9483068]  x1  via EDX

DECOMPILE (offsets annotated; truncated):
// FN 0090b200 FUN_0090b200 size=281 ret=8 callers=0 callees=9

void __thiscall FUN_0090b200(int param_1,undefined4 param_2,int param_3)

{
  undefined4 uVar1;
  
  if (param_3 != 0) {
    switch(param_2) {
    case 0x27:
    case 0x2d:
      FUN_004122a0(0);
      FUN_0090b430();
      break;
    case 0x28:
    case 0x2e:
      FUN_004122a0(0);
      FUN_0090b360();
      break;
    case 0x29:
      FUN_004122a0(0);
      uVar1 = FUN_00908a80();
      FUN_0090b890(uVar1);
      break;
    case 0x2a:
      FUN_004122a0(0);
      FUN_0090b6b0();
      break;
    case 0x2f:
    case 0x30:
    case 0x31:
    case 0x32:
    case 0x3d:
    case 0x3e:
    case 0x3f:
    case 0x40:
      uVar1 = FUN_0089c350(param_2);
      FUN_0090b9b0(uVar1);
      break;
    case 0x39:
      (**(code **)(*(int *)(param_1 + 0x18b8) + 0x3c))(0x31,1);
      break;
    case 0x3a:
      (**(code **)(*(int *)(param_1 + 0x18b8) + 0x3c))(0x32,1);
    }
  }
  FUN_0040eaa0(param_2,param_3);
  return;
}
```

---
## CARD C103
**CLAIM:** Call site at 0x009099DB in the function at 0x009098D0: bytes `e8 80 ec f3 ff` = call to 0x008480B0 [CanChooseFeat (existing DB, high)]; 0x848660(S, spell, +0x1BA9, &selected +0x1BE0): 3 stack args pushed at 0x9099BE-0x9099D7.

```
CALL WINDOW (>> = claimed address):
   009099C1  81 c2 e0 1b 00 00  ADD EDX,0x1be0
   009099C7  52                 PUSH EDX
   009099C8  8b 45 98           MOV EAX,dword ptr [EBP + -0x68]
   009099CB  0f b6 88 a9 1b 00 00 MOVZX ECX,byte ptr [EAX + 0x1ba9]
   009099D2  51                 PUSH ECX
   009099D3  0f b7 55 e4        MOVZX EDX,word ptr [EBP + -0x1c]
   009099D7  52                 PUSH EDX
   009099D8  8b 4d f0           MOV ECX,dword ptr [EBP + -0x10]
>> 009099DB  e8 80 ec f3 ff     CALL 0x00848660
   009099E0  85 c0              TEST EAX,EAX
   009099E2  0f 84 69 01 00 00  JZ 0x00909b51
   009099E8  a1 d0 b4 a1 00     MOV EAX,[0x00a1b4d0]
   009099ED  89 45 b8           MOV dword ptr [EBP + -0x48],EAX
```

---
## CARD C104
**CLAIM:** Function at 0x0089C350 is `CSWGuiLevelUpIconGrid::SelectSlotById`: finds the slot holding an id and selects it; silent no-op when absent.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0089C350, size 617, header ret=4 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x0089C350 (FUN_0089c350)
string literals (0):
direct callees (3):
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x0089A0E0  (unnamed)
  0x0073FEA0  CClientExoApp::GetSWGuiManager
struct offsets touched (top 4 of [REG + 0xNN], EBP/ESP excluded):
  +0xC [dec 12]  x17  via EAX,ECX,EDX
  +0xD [dec 13]  x15  via EAX,ECX,EDX
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x89C5D0 [dec 9029072]  x1  via EAX

DECOMPILE (offsets annotated; truncated):
// FN 0089c350 FUN_0089c350 size=617 ret=4 callers=3 callees=3

undefined4 __thiscall FUN_0089c350(int *param_1,undefined4 param_2)

{
  char cVar1;
  int iVar2;
  undefined4 uVar3;
  
  cVar1 = *(char *)((int)param_1 + 0xd);
  iVar2 = param_1[3 = byte +0xc];
  switch(param_2) {
  case 0x2f:
  case 0x3f:
    if ((char)param_1[3 = byte +0xc] != '\0') {
      uVar3 = 1;
      FUN_0073fea0(1);
      FUN_004122a0(uVar3);
      *(char *)(param_1 + 0x3 [dec 3] {int*: byte +0xc}) = (char)param_1[3 = byte +0xc] + -1;
    }
    break;
  case 0x30:
  case 0x40:
    if ((*(byte *)(param_1 + 0x3 [dec 3] {int*: byte +0xc}) < 2) &&
       (*(int *)(*(int *)(*param_1 + (uint)*(byte *)((int)param_1 + 0xd) * 4) + 0x108 +
                (uint)(byte)((char)param_1[3 = byte +0xc] + 1) * 0xb4) != -1)) {
      uVar3 = 1;
      FUN_0073fea0(1);
      FUN_004122a0(uVar3);
      *(char *)(param_1 + 0x3 [dec 3] {int*: byte +0xc}) = (char)param_1[3 = byte +0xc] + '\x01';
    }
    break;
  case 0x31:
  case 0x3d:
    if (*(char *)((int)param_1 + 0xd) == '\0') {
      *(char *)((int)param_1 + 0xd) = (char)param_1[1 = byte +0x4] + -1;
    }
    else {
      *(char *)((int)param_1 + 0xd) = *(char *)((int)param_1 + 0xd) + -1;
    }
    while (*(int *)(*(int *)(*param_1 + (uint)*(byte *)((int)param_1 + 0xd) * 4) + 0x108 +
                   (uint)*(byte *)(param_1 + 0x3 [dec 3] {int*: byte +0xc}) * 0xb4) == -1) {
      *(char *)(param_1 + 0x3 [dec 3] {int*: byte +0xc}) = (char)param_1[3 = byte +0xc] + -1;
    }
    break;
  case 0x32:
  case 0x3e:
    if ((uint)*(byte *)((int)param_1 + 0xd) == param_1[1 = byte +0x4] - 1U) {
      *(undefined1 *)((int)param_1 + 0xd) = 0;
/* ... 17 more lines truncated by --max-lines */
```

---
## CARD C105
**CLAIM:** Hook site at 0x0089AB32 inside the function at 0x0089AAA0: bytes `89 4d b8 8b 15 d0 b4 a1 00`. At this point power grid builder loop after mov ecx,[eax]: ecx = spellId (uint); window EAX-free. Consumed-exit (skip) target 0x0089AEB8 = loop increment (mov eax,[ebp-0x10]).

```
SITE WINDOW (>> = claimed address):
   0089AB1B  8b 4d d8           MOV ECX,dword ptr [EBP + -0x28]
   0089AB1E  8d 14 81           LEA EDX,[ECX + EAX*0x4]
   0089AB21  89 55 c0           MOV dword ptr [EBP + -0x40],EDX
   0089AB24  8b 45 c0           MOV EAX,dword ptr [EBP + -0x40]
   0089AB27  8b 08              MOV ECX,dword ptr [EAX]
   0089AB29  89 4d b8           MOV dword ptr [EBP + -0x48],ECX
   0089AB2C  8b 15 d0 b4 a1 00  MOV EDX,dword ptr [0x00a1b4d0]
>> 0089AB32  89 55 bc           MOV dword ptr [EBP + -0x44],EDX
   0089AB35  8b 45 b8           MOV EAX,dword ptr [EBP + -0x48]
   0089AB38  50                 PUSH EAX
   0089AB39  8b 4d bc           MOV ECX,dword ptr [EBP + -0x44]
   0089AB3C  8b 89 04 01 00 00  MOV ECX,dword ptr [ECX + 0x104]
   0089AB42  e8 09 69 e2 ff     CALL 0x006c1450
   0089AB47  89 45 cc           MOV dword ptr [EBP + -0x34],EAX
   0089AB4A  83 7d cc 00        CMP dword ptr [EBP + -0x34],0x0

EXIT WINDOW:
   0089AEAF  52                 PUSH EDX
   0089AEB0  8b 4d 84           MOV ECX,dword ptr [EBP + -0x7c]
   0089AEB3  e8 78 00 00 00     CALL 0x0089af30
>> 0089AEB8  8b 45 f0           MOV EAX,dword ptr [EBP + -0x10]
   0089AEBB  83 c0 01           ADD EAX,0x1
   0089AEBE  89 45 f0           MOV dword ptr [EBP + -0x10],EAX
   0089AEC1  e9 36 fc ff ff     JMP 0x0089aafc
```

---
## CARD C106
**CLAIM:** Function at 0x0090AE20 is `CSWGuiFeatsCharGen::InitFeatLists`: feats panel init: known/granted vectors, pick counts, then grid 0x89B600, choosable 0x90C230, repaint 0x90BD50.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0090AE20, size 115, header ret=4 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x0090AE20 (FUN_0090ae20)
string literals (0):
direct callees (3):
  0x00740350  CClientExoApp::SetPausedByCombat
  0x0073F750  ClientApp::GetGuiInGame
  0x00410530  CSWGuiManager::AddPanel
struct offsets touched (top 4 of [REG + 0xNN], EBP/ESP excluded):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x1C [dec 28]  x1  via EDX
  +0xAC [dec 172]  x1  via EAX
  +0x1BE0 [dec 7136]  x1  via EAX

DECOMPILE (offsets annotated; truncated):
// FN 0090ae20 FUN_0090ae20 size=115 ret=4 callers=1 callees=3

void __fastcall FUN_0090ae20(int param_1)

{
  int iVar1;
  
  if ((*(uint *)(param_1 + 0x1be0) & 1) == 0) {
    FUN_00740350(0,2,0);
  }
  iVar1 = FUN_0073f750();
  FUN_00410530(*(undefined4 *)(iVar1 + 0xac),1,1);
  return;
}
```

---
## CARD C107
**CLAIM:** Function at 0x0090B890 is `CSWGuiFeatsCharGen::OnSlotClicked`: feats slot click: refusal popups strref 0xA4C8 (state 4) / 0xA4C7 / 0xA4C6 (state 2/3).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0090B890, size 263, header ret=4 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x0090B890 (FUN_0090b890)
string literals (0):
direct callees (8):
  0x0090C120  (unnamed)
  0x0090BF90  (unnamed)
  0x0090BD50  (unnamed)
  0x0073F750  ClientApp::GetGuiInGame
  0x0075BB00  (unnamed)
  0x0090BCB0  (unnamed)
  0x0075CA30  (unnamed)
  0x00410530  CSWGuiManager::AddPanel
struct offsets touched (top 5 of [REG + 0xNN], EBP/ESP excluded):
  +0x4 [dec 4]  x1  via EDX
  +0x1C [dec 28]  x1  via ECX
  +0x70 [dec 112]  x1  via EDX
  +0xA0 [dec 160]  x1  via ECX
  +0x1BDE [dec 7134]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 0090b890 FUN_0090b890 size=263 ret=4 callers=2 callees=8

void __thiscall FUN_0090b890(int param_1,undefined2 param_2)

{
  int *piVar1;
  int iVar2;
  int local_c;
  undefined1 local_5;
  
  FUN_0090bcb0(param_2,&local_5);
  local_c = 0;
  switch(local_5) {
  case 0:
    if (*(char *)(param_1 + 0x1bde) == '\0') {
      local_c = 0xa622;
    }
    else {
      FUN_0090bf90(param_2);
      FUN_0090bd50();
    }
    break;
  case 1:
    FUN_0090c120(param_2);
    FUN_0090bd50();
    break;
  case 2:
    local_c = 0xa4c7;
    break;
  case 3:
    local_c = 0xa4c6;
    break;
  case 4:
    local_c = 0xa4c8;
  }
  if (local_c != 0) {
    iVar2 = FUN_0073f750();
    piVar1 = *(int **)(iVar2 + 0xa0);
    FUN_0075bb00(0);
    FUN_0075ca30(0,0,0);
    (**(code **)(*piVar1 + 0x70))(local_c);
    FUN_00410530(piVar1,1,1);
  }
  return;
}
```

---
## CARD C108
**CLAIM:** Function at 0x0090F610 is `CSWGuiSkillsCharGen::RefreshRowStyling`: skills row dimming: disabled = !(skillRow+0x1C bit4), colour DAT_00A7FD4C vs DAT_00A7FDAC.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0090F610, size 984, header ret=4 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x0090F610 (FUN_0090f610)
string literals (0):
direct callees (11):
  0x00734F60  CExoString::CompareNoCase(char*)
  0x00417180  (unnamed)
  0x00733700  CExoString::Constructor_5
  0x00733780  CExoString::~CExoString
  0x00846350  (unnamed)
  0x009102A0  (unnamed)
  0x00416EA0  (unnamed)
  0x00416E30  CSWGuiTextParams::SetText
  0x00417010  (unnamed)
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x009103A0  (unnamed)
struct offsets touched (top 7 of [REG + 0xNN], EBP/ESP excluded):
  +0x4CEC [dec 19692]  x8  via EAX,ECX,EDX
  +0x38 [dec 56]  x4  via EAX,ECX
  +0x1C [dec 28]  x2  via EAX,ECX
  +0x68 [dec 104]  x2  via EDX
  +0x4CF8 [dec 19704]  x2  via ECX,EDX
  +0x12C [dec 300]  x1  via ECX
  +0x310 [dec 784]  x1  via EAX

DECOMPILE (offsets annotated; truncated):
// FN 0090f610 FUN_0090f610 size=984 ret=4 callers=2 callees=11

void __thiscall FUN_0090f610(int param_1,int param_2)

{
  bool bVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  undefined1 local_24 [8];
  int local_1c;
  int local_18;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00958a08;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  for (local_14 = 0; (int)local_14 < 8; local_14 = local_14 + 1) {
    local_1c = 0;
    if ((*(uint *)(param_1 + 0x4cf8) >> 1 & 1) == 0) {
      local_18 = (local_14 & 0xffff) * 0x20 + *(int *)(DAT_00a1b4d0 + 0x12c [dec 300]);
      bVar1 = false;
      if (local_14 == 2) {
        FUN_0077d800(uVar2);
        iVar3 = FUN_00734f60(&DAT_00995154);
        if (iVar3 != 0) {
          bVar1 = true;
        }
      }
      if ((*(uint *)(local_18 + 0x1c) >> 4 & 1) == 0) {
        local_1c = 1;
      }
      else if ((((*(uint *)(param_1 + 0x4cf8) >> 2 & 1) != 0) &&
               ((*(uint *)(local_18 + 0x1c) >> 5 & 1) == 0)) && (!bVar1)) {
        local_1c = 1;
      }
    }
    if (param_2 == param_1 + 0x1928 + local_14 * 0x1d0) {
      *(uint *)(param_1 + 0x4cec) = local_14;
      if (local_1c == 0) {
        FUN_00417010(&DAT_00a7fdac);
        FUN_00417010(&DAT_00a7fdac);
      }
      else {
/* ... 40 more lines truncated by --max-lines */
```

---
## CARD C109
**CLAIM:** Function at 0x0090AEA0 is `CSWGuiFeatsCharGen::InitFeatLists`: feats panel init: known/granted vectors, pick counts, then grid 0x89B600, choosable 0x90C230, repaint 0x90BD50.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0090AEA0, size 861, header ret=0 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x0090AEA0 (FUN_0090aea0)
string literals (0):
direct callees (16):
  0x00847E50  CSWCLevelUpStats::CalcNumberFeats
  0x0073F750  ClientApp::GetGuiInGame
  0x0089B600  (unnamed)
  0x0090C230  (unnamed)
  0x0084A640  (unnamed)
  0x0064DC30  (unnamed)
  0x008470A0  (unnamed)
  0x0089CB60  (unnamed)
  0x0090BD50  (unnamed)
  0x008450E0  (unnamed)
  0x00593450  CExoArrayList<uint16>::Add
  0x0049B840  (unnamed)
  0x00416EA0  (unnamed)
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x0090B430  CSWGuiFeatsCharGen::OnAcceptButton
  0x00844560  (unnamed)
struct offsets touched (top 11 of [REG + 0xNN], EBP/ESP excluded):
  +0x1BE0 [dec 7136]  x10  via EAX,ECX,EDX
  +0x1BDA [dec 7130]  x4  via EAX,ECX
  +0x1BDB [dec 7131]  x4  via EAX,ECX,EDX
  +0x1BDE [dec 7134]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x68 [dec 104]  x1  via EAX
  +0xAC [dec 172]  x1  via EDX
  +0xBC [dec 188]  x1  via EDX
  +0x310 [dec 784]  x1  via ECX
  +0x1BDC [dec 7132]  x1  via EDX
  +0x1BDD [dec 7133]  x1  via EDX

DECOMPILE (offsets annotated; truncated):
// FN 0090aea0 FUN_0090aea0 size=861 ret=0 callers=1 callees=16

void __fastcall FUN_0090aea0(int param_1)

{
  short sVar1;
  undefined2 uVar2;
  int iVar3;
  char cVar4;
  int local_24;
  uint local_20;
  uint local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009810e8;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_14 = *(int *)(*(int *)(param_1 + 0x68) + 0x310);
  if (((*(uint *)(param_1 + 0x1be0) & 1) == 0) && ((*(uint *)(param_1 + 0x1be0) >> 1 & 1) == 0)) {
    FUN_00844560(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  }
  for (local_18 = 0; (int)local_18 < (int)(*(uint *)(local_14 + 0xbc) & 0xffff);
      local_18 = local_18 + 1) {
    sVar1 = FUN_008450e0(local_18 & 0xffff);
    if ((sVar1 != 0xcf) && (sVar1 = FUN_008450e0(local_18 & 0xffff), sVar1 != 0xee)) {
      uVar2 = FUN_008450e0(local_18 & 0xffff);
      FUN_00593450(uVar2);
    }
  }
  *(uint *)(param_1 + 0x1be0) = *(uint *)(param_1 + 0x1be0) & 0xfffffffb;
  if ((*(uint *)(param_1 + 0x1be0) >> 1 & 1) == 0) {
    FUN_0084a640(0);
    local_8 = 0;
    if ((*(uint *)(param_1 + 0x1be0) & 1) == 0) {
      FUN_008470a0(&local_24,1);
    }
    else {
      FUN_008470a0(&local_24,0);
    }
    if (0 < (int)local_20) {
      for (local_18 = 0; (int)local_18 < (int)local_20; local_18 = local_18 + 1) {
        FUN_00593450(*(undefined2 *)(local_24 + local_18 * 4));
      }
      while (local_18 = local_20 - 1, -1 < (int)local_18) {
        iVar3 = FUN_0064dc30(*(undefined2 *)(local_24 + local_18 * 4));
/* ... 47 more lines truncated by --max-lines */
```

---
## CARD C110
**CLAIM:** Class `CSWGuiPowersLevelUp` has vftable 0x009AA434; its constructor is 0x00909E00.

```
ASM lines of ctor 0x00909E00 containing a 0x9AAxxx constant:
00909e4a  c70134a49a00          MOV dword ptr [ECX],0x9aa434
0090a0d2  6824a49a00            PUSH 0x9aa424
0090a12c  682ca39a00            PUSH 0x9aa32c
0090a169  6818a39a00            PUSH 0x9aa318
0090a1a5  6804a49a00            PUSH 0x9aa404
0090a1e2  68e4a39a00            PUSH 0x9aa3e4

ANCHORS of ctor:
anchors for 0x00909E00 (FUN_00909e00)
string literals (14):
  0x009AA424  at 0090a0d2  'FTCHRGEN_p'
  0x009A969C  at 0090a0f2  'MAIN_TITLE_LBL'
  0x009AA32C  at 0090a12c  'SUB_TITLE_LBL'
  0x009AA318  at 0090a169  'REMAINING_BOX_LBL'
  0x009AA404  at 0090a1a5  'STD_REMAINING_SELECTIONS_LBL'
  0x009AA3E4  at 0090a1e2  'STD_SELECTIONS_REMAINING_LBL'
  0x009A1810  at 0090a21f  'LB_DESC'
  0x009AA3D8  at 0090a268  'LB_FEATS'
  0x0099D72C  at 0090a2a4  'LBL_NAME'
  0x009A6968  at 0090a2e1  'BTN_ACCEPT'

VTABLE DUMP:
0x9AA434 (28 slots)
  slot 0 +0x0 = 0x90A970
  slot 15 +0x3C = 0x90B200
  slot 18 +0x48 = 0x90AD90
```


# BATCH 6 (cards C111-C129)

---
## CARD C111
**CLAIM:** Function at 0x0089AF30 is `CSWGuiLevelUpIconGrid::AddPowerRow`: allocs grid row for (spell, tier2, tier3, flag); power twin of 0x89BED0.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0089AF30, size 463, header ret=16 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x0089AF30 (FUN_0089af30)
string literals (0):
direct callees (9):
  0x00710EB0  (unnamed)
  0x007107F0  (unnamed)
  0x006C1450  (unnamed)
  0x009196EE  __security_check_cookie
  0x00416090  (unnamed)
  0x00899930  (unnamed)
  0x0083EA60  CExoArrayList<uint32>::Add
  0x00919723  operator new
  0x00710CA0  (unnamed)
struct offsets touched (top 2 of [REG + 0xNN], EBP/ESP excluded):
  +0x4 [dec 4]  x1  via EDX
  +0x104 [dec 260]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 0089af30 FUN_0089af30 size=463 ret=16 callers=1 callees=9

void FUN_0089af30(int param_1,undefined4 param_2,undefined4 param_3,int param_4)

{
  uint uVar1;
  int *piVar2;
  undefined4 extraout_var;
  undefined4 uVar3;
  undefined4 local_84;
  undefined4 local_80;
  undefined4 local_7c;
  undefined4 local_78;
  undefined1 local_74 [16];
  undefined1 local_64 [16];
  undefined1 local_54 [16];
  int local_44;
  int *local_40;
  int local_3c;
  undefined1 local_38 [16];
  uint local_28;
  int *local_24;
  int local_20[4 = byte +0x10];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00976b5b;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_28 = uVar1;
  FUN_007107f0(uVar1);
  uVar3 = extraout_var;
  local_44 = FUN_00919723(0x324);
  local_8 = 0;
  if (local_44 == 0) {
    piVar2 = (int *)0x0;
  }
  else {
    piVar2 = (int *)FUN_00899930();
  }
  local_8 = 0xffffffff;
  local_20[0] = param_1;
  local_20[1 = byte +0x4] = param_2;
  local_20[2 = byte +0x8] = param_3;
  local_40 = piVar2;
  local_24 = piVar2;
/* ... 26 more lines truncated by --max-lines */
```

---
## CARD C112
**CLAIM:** Function at 0x0089C220 is `CSWGuiLevelUpIconGrid::SetAllSlotStates`: sets every slot of every row to the given state (called with 3 = dimmed).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0089C220, size 104, header ret=4 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x0089C220 (FUN_0089c220)
string literals (0):
direct callees (1):
  0x0089A160  (unnamed)
struct offsets touched (top 1 of [REG + 0xNN], EBP/ESP excluded):
  +0x4 [dec 4]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 0089c220 FUN_0089c220 size=104 ret=4 callers=4 callees=1

void __thiscall FUN_0089c220(int param_1,undefined1 param_2)

{
  undefined4 local_c;
  undefined4 local_8;
  
  for (local_8 = 0; local_8 < *(int *)(param_1 + 4); local_8 = local_8 + 1) {
    for (local_c = 0; (int)local_c < 3; local_c = local_c + 1) {
      FUN_0089a160(local_c & 0xff,param_2);
    }
  }
  return;
}
```

---
## CARD C113
**CLAIM:** Function at 0x0090B360 is `CSWGuiFeatsCharGen::OnInputEvent`: vtable slot 15 event switch: 0x27/0x2D accept, 0x28/0x2E back, 0x29 click, 0x2A recommended, arrows.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0090B360, size 203, header ret=0 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x0090B360 (FUN_0090b360)
string literals (0):
direct callees (5):
  0x009150E0  (unnamed)
  0x004109C0  CSWGuiManager::PopModalPanel
  0x00904780  (unnamed)
  0x0040FE60  CSWGuiPanel::SetVisible
  0x00844560  (unnamed)
struct offsets touched (top 6 of [REG + 0xNN], EBP/ESP excluded):
  +0x48 [dec 72]  x4  via EAX,ECX
  +0x6C [dec 108]  x4  via EAX,ECX,EDX
  +0x68 [dec 104]  x3  via EAX,ECX
  +0x310 [dec 784]  x2  via ECX,EDX
  +0x1BE0 [dec 7136]  x2  via ECX
  +0x1C [dec 28]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 0090b360 FUN_0090b360 size=203 ret=0 callers=1 callees=5

void __fastcall FUN_0090b360(int param_1)

{
  if (((*(int *)(param_1 + 0x68) != 0) && (*(int *)(*(int *)(param_1 + 0x68) + 0x310) != 0)) &&
     ((*(uint *)(param_1 + 0x1be0) & 1) == 0)) {
    FUN_00844560(param_1,*(undefined4 *)(*(int *)(param_1 + 0x68) + 0x310));
  }
  if (*(int *)(param_1 + 0x6c) != 0) {
    FUN_004109c0();
    FUN_0040fe60(1);
    *(uint *)(param_1 + 0x48) = *(uint *)(param_1 + 0x48) & 0xffffff7f;
    *(uint *)(param_1 + 0x48) = *(uint *)(param_1 + 0x48) & 0xfffffcff | 0x200;
    if ((*(uint *)(param_1 + 0x1be0) & 1) == 0) {
      FUN_009150e0();
    }
    else {
      FUN_00904780();
    }
  }
  return;
}
```

---
## CARD C114
**CLAIM:** Function at 0x00909E00 is `CSWGuiPowersLevelUp::Constructor`: powers level-up panel ctor (grid ptr at +0x1BF8).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x00909E00, size 2914, header ret=16 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x00909E00 (FUN_00909e00)
string literals (14):
  0x009AA424  at 0090a0d2  'FTCHRGEN_p'
  0x009A969C  at 0090a0f2  'MAIN_TITLE_LBL'
  0x009AA32C  at 0090a12c  'SUB_TITLE_LBL'
  0x009AA318  at 0090a169  'REMAINING_BOX_LBL'
  0x009AA404  at 0090a1a5  'STD_REMAINING_SELECTIONS_LBL'
  0x009AA3E4  at 0090a1e2  'STD_SELECTIONS_REMAINING_LBL'
  0x009A1810  at 0090a21f  'LB_DESC'
  0x009AA3D8  at 0090a268  'LB_FEATS'
  0x0099D72C  at 0090a2a4  'LBL_NAME'
  0x009A6968  at 0090a2e1  'BTN_ACCEPT'
  0x009AA3C8  at 0090a327  'BTN_RECOMMENDED'
  0x009AA3BC  at 0090a3b2  'BTN_SELECT'
  0x0099E548  at 0090a3f8  'LBL_BAR1'
  0x0099E53C  at 0090a43d  'LBL_BAR2'
direct callees (25):
  0x0041A510  (unnamed)
  0x0091D67E  (unnamed)
  0x00760EA0  (unnamed)
  0x00733DF0  CExoString::operator+
  0x005933B0  (unnamed)
  0x0091C860  _ftol2_sse
  0x0041CE90  (unnamed)
  0x00419740  (unnamed)
  0x0040E5A0  CSWGuiPanel::Constructor
  0x007337C0  CExoString::operator=
  0x0090AEA0  (unnamed)
  0x0040F5A0  CSWGuiPanel::StopLoadFromLayout
  0x00733780  CExoString::~CExoString
  0x00733570  CExoString::CExoString_char
  0x00416EA0  (unnamed)
  0x0040F620  CSWGuiPanel::InitControl
  0x00418AF0  CSWGuiControl::AddEvent

DECOMPILE (offsets annotated; truncated):
// FN 00909e00 FUN_00909e00 size=2914 ret=16 callers=3 callees=25

undefined4 * __thiscall
FUN_00909e00(undefined4 *param_1,undefined4 param_2,undefined4 param_3,uint param_4,uint param_5)

{
  uint uVar1;
  undefined4 uVar2;
  float10 fVar3;
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
  undefined1 local_40 [16];
  int local_30;
  float local_2c;
  undefined1 local_28 [8];
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00980e9e;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_0040e5a0(param_2);
/* ... 202 more lines truncated by --max-lines */
```

---
## CARD C115
**CLAIM:** Function at 0x0090C6A0 is `CSWGuiSkillsCharGen::Constructor`: skills level-up panel ctor (8 fixed skill rows).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0090C6A0, size 7526, header ret=12 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x0090C6A0 (FUN_0090c6a0)
string literals (46):
  0x009AA700  at 0090cac9  'SKCHRGEN_p'
  0x009A969C  at 0090cae9  'MAIN_TITLE_LBL'
  0x009AA32C  at 0090cb42  'SUB_TITLE_LBL'
  0x009AA318  at 0090cb7e  'REMAINING_BOX_LBL'
  0x009AA2FC  at 0090cbbb  'REMAINING_SELECTIONS_LBL'
  0x009AA2E0  at 0090cbf8  'SELECTIONS_REMAINING_LBL'
  0x009AA6F4  at 0090cc34  'COST_LBL'
  0x009AA6E4  at 0090cc71  'COST_POINTS_LBL'
  0x009AA6D4  at 0090ccae  'CLASSSKL_LBL'
  0x009A1810  at 0090ccea  'LB_DESC'
  0x009A6968  at 0090cd3a  'BTN_ACCEPT'
  0x009AA3C8  at 0090cd80  'BTN_RECOMMENDED'
  0x0099E548  at 0090ce0b  'LBL_BAR1'
  0x0099E53C  at 0090ce51  'LBL_BAR2'
  0x009AA6BC  at 0090cedb  'COMPUTER_USE_POINTS_BTN'
  0x009AA6A8  at 0090cf20  'COMPUTER_USE_LBL'
  0x009AA698  at 0090cf66  'COM_MINUS_BTN'
  0x009AA688  at 0090cfac  'COM_PLUS_BTN'
  0x009AA670  at 0090d059  'DEMOLITIONS_POINTS_BTN'
  0x009AA660  at 0090d09f  'DEMOLITIONS_LBL'
  0x009AA650  at 0090d0e5  'DEM_MINUS_BTN'
  0x009AA640  at 0090d12a  'DEM_PLUS_BTN'
  0x009AA62C  at 0090d1dc  'STEALTH_POINTS_BTN'
  0x009AA620  at 0090d222  'STEALTH_LBL'
  0x009AA610  at 0090d267  'STE_MINUS_BTN'
  0x009AA600  at 0090d2ad  'STE_PLUS_BTN'
  0x009AA5EC  at 0090d35f  'PERSUADE_POINTS_BTN'
  0x009AA5DC  at 0090d3a4  'PERSUADE_LBL'
  0x009AA5CC  at 0090d3ea  'PER_MINUS_BTN'
  0x009AA5BC  at 0090d430  'PER_PLUS_BTN'
  0x009AA5A8  at 0090d4e0  'REPAIR_POINTS_BTN'
  0x009AA59C  at 0090d526  'REPAIR_LBL'

DECOMPILE (offsets annotated; truncated):
// FN 0090c6a0 FUN_0090c6a0 size=7526 ret=12 callers=2 callees=30

undefined4 * __thiscall
FUN_0090c6a0(undefined4 *param_1,undefined4 param_2,undefined4 param_3,uint param_4)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  int local_2cc;
  char local_296;
  char local_28a;
  char local_27a;
  undefined1 local_26a;
  undefined1 local_200 [8];
  undefined1 local_1f8 [8];
  undefined1 local_1f0 [8];
  undefined1 local_1e8 [8];
  undefined1 local_1e0 [8];
  undefined1 local_1d8 [8];
  undefined1 local_1d0 [8];
  undefined1 local_1c8 [8];
  undefined1 local_1c0 [8];
  undefined1 local_1b8 [8];
  undefined1 local_1b0 [8];
  undefined1 local_1a8 [8];
  undefined1 local_1a0 [8];
  undefined1 local_198 [8];
  undefined1 local_190 [8];
  undefined1 local_188 [8];
  undefined1 local_180 [8];
  undefined1 local_178 [8];
  undefined1 local_170 [8];
  undefined1 local_168 [8];
  undefined1 local_160 [8];
  undefined1 local_158 [8];
  undefined1 local_150 [8];
  undefined1 local_148 [8];
  undefined1 local_140 [8];
  undefined1 local_138 [8];
  undefined1 local_130 [8];
  undefined1 local_128 [8];
  undefined1 local_120 [8];
  undefined1 local_118 [8];
  undefined1 local_110 [8];
  undefined1 local_108 [8];
  undefined1 local_100 [8];
  undefined1 local_f8 [8];
  undefined1 local_f0 [8];
/* ... 500 more lines truncated by --max-lines */
```

---
## CARD C116
**CLAIM:** Function at 0x0089B100 is `CSWGuiLevelUpIconGrid::BuildSortedPowerList`: sorts spells.2da rows (type +0x14C in {1,6}, name strref != 0) by multi-digit key, caches at +0x1048.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0089B100, size 535, header ret=4 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x0089B100 (FUN_0089b100)
string literals (0):
direct callees (5):
  0x0089CD50  (unnamed)
  0x005934C0  (unnamed)
  0x00593450  CExoArrayList<uint16>::Add
  0x0073F750  ClientApp::GetGuiInGame
  0x006A20F0  (unnamed)
struct offsets touched (top 5 of [REG + 0xNN], EBP/ESP excluded):
  +0x4 [dec 4]  x4  via EAX,EDX
  +0x38 [dec 56]  x2  via ECX,EDX
  +0x3A [dec 58]  x2  via ECX,EDX
  +0x8 [dec 8]  x1  via EDX
  +0x11C [dec 284]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 0089b100 FUN_0089b100 size=535 ret=4 callers=2 callees=5

void __thiscall FUN_0089b100(undefined4 param_1,int *param_2)

{
  bool bVar1;
  int iVar2;
  int local_24;
  uint local_20;
  uint local_1c;
  uint local_14;
  
  iVar2 = FUN_0073f750(param_1);
  if ((iVar2 + 0x103c == 0) || (*(int *)(iVar2 + 0x1040) < 1)) {
    for (local_14 = 0; (int)local_14 < (int)(uint)*(ushort *)(DAT_00a1b4d0 + 0x11c);
        local_14 = local_14 + 1) {
      iVar2 = FUN_006a20f0(local_14 & 0xffff);
      if ((((local_14 != 0xcf) && (local_14 != 0xee)) && (iVar2 != 0)) && (*(int *)(iVar2 + 8) != 0)
         ) {
        bVar1 = false;
        if (*(short *)(iVar2 + 0x38) == -1) {
          local_1c = 0;
        }
        else if (*(short *)(iVar2 + 0x3a) == -1) {
          local_1c = 1;
        }
        else {
          local_1c = 2;
        }
        for (local_24 = 0; local_24 < param_2[1 = byte +0x4]; local_24 = local_24 + 1) {
          iVar2 = FUN_006a20f0(*(undefined2 *)(*param_2 + local_24 * 2));
          if (iVar2 != 0) {
            if (*(short *)(iVar2 + 0x38) == -1) {
              local_20 = 0;
            }
            else if (*(short *)(iVar2 + 0x3a) == -1) {
              local_20 = 1;
            }
            else {
              local_20 = 2;
            }
            if (local_1c < local_20) {
              bVar1 = true;
            }
            if (bVar1) {
              FUN_005934c0(local_14 & 0xffff,local_24);
              break;
            }
          }
/* ... 14 more lines truncated by --max-lines */
```

---
## CARD C117
**CLAIM:** Function at 0x00848660 is `CSWCLevelUpStats::CanChooseSpell`: power availability: spell type in {1,5}, class has powers, picks left, not known, not selected, prereq 0x846DD0.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x00848660, size 326, header ret=12 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x00848660 (FUN_00848660)
string literals (0):
direct callees (5):
  0x006C1450  (unnamed)
  0x0084A170  (unnamed)
  0x00844E40  CSWSCreatureStats::GetClassByPosition
  0x006C10A0  (unnamed)
  0x00846DD0  (unnamed)
struct offsets touched (top 4 of [REG + 0xNN], EBP/ESP excluded):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x14C [dec 332]  x2  via EAX
  +0x32 [dec 50]  x1  via EAX
  +0x104 [dec 260]  x1  via EDX

DECOMPILE (offsets annotated; truncated):
// FN 00848660 FUN_00848660 size=326 ret=12 callers=2 callees=5

undefined4 __thiscall FUN_00848660(int param_1,int param_2,byte param_3,int *param_4)

{
  undefined1 uVar1;
  char cVar2;
  int iVar3;
  undefined4 uVar4;
  int iVar5;
  ushort local_10;
  
  cVar2 = *(char *)(param_1 + 0x32);
  iVar3 = FUN_006c1450(param_2);
  if (iVar3 == 0) {
    uVar4 = 0;
  }
  else if ((*(char *)(iVar3 + 0x14c) == '\x01') || (*(char *)(iVar3 + 0x14c) == '\x05')) {
    uVar1 = FUN_00844e40(cVar2 + -1);
    cVar2 = FUN_006c10a0(uVar1);
    if (cVar2 == -1) {
      uVar4 = 0;
    }
    else if (param_3 == 0) {
      uVar4 = 0;
    }
    else {
      iVar5 = FUN_0084a170(0,param_2);
      if (iVar5 == 0) {
        for (local_10 = 0; (int)(uint)local_10 < param_4[1 = byte +0x4]; local_10 = local_10 + 1) {
          if (param_2 == *(int *)(*param_4 + (uint)local_10 * 4)) {
            return 0;
          }
        }
        iVar3 = FUN_00846dd0(iVar3,param_4);
        if (iVar3 == 0) {
          uVar4 = 0;
        }
        else if ((uint)param_3 == param_4[1 = byte +0x4] || (int)((uint)param_3 - param_4[1 = byte +0x4]) < 0) {
          uVar4 = 0;
        }
        else {
          uVar4 = 1;
        }
      }
      else {
        uVar4 = 0;
      }
    }
/* ... 6 more lines truncated by --max-lines */
```

---
## CARD C118
**CLAIM:** Function at 0x0089BED0 is `CSWGuiLevelUpIconGrid::AddPowerRow`: allocs grid row for (spell, tier2, tier3, flag); power twin of 0x89BED0.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0089BED0, size 403, header ret=12 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x0089BED0 (FUN_0089bed0)
string literals (0):
direct callees (11):
  0x00710EB0  (unnamed)
  0x007107F0  (unnamed)
  0x009196EE  __security_check_cookie
  0x00416090  (unnamed)
  0x0091C860  _ftol2_sse
  0x00899930  (unnamed)
  0x0083EA60  CExoArrayList<uint32>::Add
  0x00919723  operator new
  0x00710CA0  (unnamed)
  0x00479AB0  (unnamed)
  0x006A20F0  (unnamed)
struct offsets touched (top 1 of [REG + 0xNN], EBP/ESP excluded):
  +0x4 [dec 4]  x1  via EAX

DECOMPILE (offsets annotated; truncated):
// FN 0089bed0 FUN_0089bed0 size=403 ret=12 callers=2 callees=11

void FUN_0089bed0(int param_1,undefined4 param_2,undefined4 param_3)

{
  uint uVar1;
  int *piVar2;
  float10 fVar3;
  undefined4 extraout_var;
  undefined4 uVar4;
  undefined4 local_78;
  undefined4 local_74;
  undefined4 local_70;
  undefined4 local_6c;
  undefined1 local_68 [16];
  undefined1 local_58 [16];
  int local_48;
  int *local_44;
  undefined1 local_40 [16];
  uint local_30;
  float local_2c;
  int *local_28;
  int local_24[5 = byte +0x14];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00976c7b;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_30 = uVar1;
  FUN_007107f0(uVar1);
  uVar4 = extraout_var;
  local_48 = FUN_00919723(0x324);
  local_8 = 0;
  if (local_48 == 0) {
    piVar2 = (int *)0x0;
  }
  else {
    piVar2 = (int *)FUN_00899930();
  }
  local_8 = 0xffffffff;
  local_24[0] = param_1;
  local_24[1 = byte +0x4] = param_2;
  local_24[2 = byte +0x8] = param_3;
  local_44 = piVar2;
  local_28 = piVar2;
/* ... 23 more lines truncated by --max-lines */
```

---
## CARD C119
**CLAIM:** Function at 0x0090EAC0 is `CSWGuiSkillsCharGen::OnRankUpButton`: skills rank-up button handler: cost via 0x846350, rank change via 0x846690.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0090EAC0, size 786, header ret=0 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x0090EAC0 (FUN_0090eac0)
string literals (0):
direct callees (5):
  0x00733700  CExoString::Constructor_5
  0x00733780  CExoString::~CExoString
  0x008468C0  (unnamed)
  0x0090F610  (unnamed)
  0x00416E30  CSWGuiTextParams::SetText
struct offsets touched (top 9 of [REG + 0xNN], EBP/ESP excluded):
  +0x4CEC [dec 19692]  x13  via EAX,ECX,EDX
  +0x68 [dec 104]  x5  via EAX,ECX,EDX
  +0x310 [dec 784]  x5  via EAX,ECX,EDX
  +0x48 [dec 72]  x4  via EAX,EDX
  +0x90 [dec 144]  x3  via EAX,ECX
  +0x123 [dec 291]  x3  via ECX,EDX
  +0x4CE8 [dec 19688]  x2  via EAX,ECX
  +0x40 [dec 64]  x1  via EDX
  +0xB6 [dec 182]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 0090eac0 FUN_0090eac0 size=786 ret=0 callers=1 callees=5

void __fastcall FUN_0090eac0(int param_1)

{
  int iVar1;
  char local_55;
  char local_41;
  char local_29;
  undefined1 local_20 [8];
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00981820;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (*(ushort *)(param_1 + 0x4cec) < (ushort)*(byte *)(DAT_00a1b4d0 + 0x123)) {
    local_29 = *(char *)((uint)*(ushort *)(param_1 + 0x4cec) +
                        *(int *)(*(int *)(*(int *)(param_1 + 0x68) + 0x310) + 0x90));
  }
  else {
    local_29 = '\0';
  }
  if ((int)local_29 != (uint)*(byte *)(param_1 + 0x4cf0 + *(int *)(param_1 + 0x4cec))) {
    FUN_008468c0(*(undefined2 *)(param_1 + 0x4cec),
                 *(undefined1 *)(param_1 + 0x4cf0 + *(int *)(param_1 + 0x4cec)));
    *(uint *)(param_1 + 0x4ce8) =
         (uint)*(ushort *)(*(int *)(*(int *)(param_1 + 0x68) + 0x310) + 0xb6);
    if (*(ushort *)(param_1 + 0x4cec) < (ushort)*(byte *)(DAT_00a1b4d0 + 0x123)) {
      local_41 = *(char *)((uint)*(ushort *)(param_1 + 0x4cec) +
                          *(int *)(*(int *)(*(int *)(param_1 + 0x68) + 0x310) + 0x90));
    }
    else {
      local_41 = '\0';
    }
    FUN_00733700((int)local_41);
    local_8 = 0;
    FUN_00416e30(local_20);
    FUN_00733700(*(undefined4 *)(param_1 + 0x4ce8));
    local_8 = CONCAT31(local_8._1_3_,1);
    FUN_00416e30(local_18);
    FUN_0090f610(param_1 + 0x1928 + *(int *)(param_1 + 0x4cec) * 0x1d0);
    iVar1 = param_1 + 0x2d18 + *(int *)(param_1 + 0x4cec) * 0x1d0;
    *(uint *)(iVar1 + 0x48) = *(uint *)(iVar1 + 0x48) & 0xfffffffd | 2;
    if (*(ushort *)(param_1 + 0x4cec) < (ushort)*(byte *)(DAT_00a1b4d0 + 0x123)) {
      local_55 = *(char *)((uint)*(ushort *)(param_1 + 0x4cec) +
/* ... 18 more lines truncated by --max-lines */
```

---
## CARD C120
**CLAIM:** Function at 0x008F7FE0 is `CSWGuiLevelUpCharGen::Constructor`: level-up outer panel ctor (+0x68 temp baseline creature, +0x2CB4 creature id).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x008F7FE0, size 141, header ret=0 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x008F7FE0 (FUN_008f7fe0)
string literals (0):
direct callees (3):
  0x00844E80  (unnamed)
  0x00846AD0  (unnamed)
  0x0073F550  CClientExoApp::GetCreatureByGameObjectID
struct offsets touched (top 4 of [REG + 0xNN], EBP/ESP excluded):
  +0x310 [dec 784]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x32 [dec 50]  x1  via EAX
  +0x2CB4 [dec 11444]  x1  via EDX

DECOMPILE (offsets annotated; truncated):
// FN 008f7fe0 FUN_008f7fe0 size=141 ret=0 callers=1 callees=3

void __fastcall FUN_008f7fe0(int param_1)

{
  int iVar1;
  uint uVar2;
  char cVar3;
  
  iVar1 = FUN_0073f550(*(undefined4 *)(param_1 + 0x2cb4));
  if (iVar1 != 0) {
    cVar3 = *(char *)(*(int *)(iVar1 + 0x310) + 0x32) + -1;
    uVar2 = FUN_00844e80(cVar3);
    FUN_00846ad0(cVar3,(uVar2 & 0xff) + 1);
  }
  return;
}
```

---
## CARD C121
**CLAIM:** Function at 0x008480B0 is `CSWCLevelUpStats::CanChooseSpell`: power availability: spell type in {1,5}, class has powers, picks left, not known, not selected, prereq 0x846DD0.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x008480B0, size 1447, header ret=16 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x008480B0 (FUN_008480b0)
string literals (2):
  0x00995164  at 0084817a  'baodur'
  0x00995188  at 00848222  'kreia'
direct callees (10):
  0x008450E0  (unnamed)
  0x0084A640  (unnamed)
  0x00846B20  (unnamed)
  0x008487B0  (unnamed)
  0x00840EF0  (unnamed)
  0x00847A30  (unnamed)
  0x00733780  CExoString::~CExoString
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x00734F60  CExoString::CompareNoCase(char*)
  0x006A20F0  (unnamed)
struct offsets touched (top 4 of [REG + 0xNN], EBP/ESP excluded):
  +0x4 [dec 4]  x5  via EAX,ECX,EDX
  +0x49 [dec 73]  x1  via EDX
  +0x4E [dec 78]  x1  via EDX
  +0xBC [dec 188]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 008480b0 FUN_008480b0 size=1447 ret=16 callers=2 callees=10

undefined4 __thiscall FUN_008480b0(int param_1,short param_2,char param_3,char param_4,int *param_5)

{
  short sVar1;
  uint uVar2;
  undefined4 uVar3;
  int iVar4;
  char local_a8;
  char local_9c;
  ushort local_78;
  undefined1 local_50 [11];
  char local_45;
  undefined1 local_44 [10];
  char local_3a;
  char local_39;
  int local_38;
  ushort local_34;
  char local_2d;
  int local_2c;
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00970eaa;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_20 = 0;
  local_38 = 0;
  if ((param_2 == 0xcf) || (param_2 == 0xee)) {
    uVar3 = 0;
  }
  else {
    local_18 = FUN_006a20f0(param_2);
    if (local_18 == 0) {
      uVar3 = 0;
    }
    else if ((param_3 == '\0') && (param_4 == '\0')) {
      uVar3 = 0;
    }
    else {
      FUN_00847a30(local_44);
/* ... 150 more lines truncated by --max-lines */
```

---
## CARD C122
**CLAIM:** Function at 0x0089B100 is `CSWGuiLevelUpIconGrid::BuildSortedFeatList`: sorts feat ids by tier then id, caches at GUI state +0x103C.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0089B100, size 535, header ret=4 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x0089B100 (FUN_0089b100)
string literals (0):
direct callees (5):
  0x0089CD50  (unnamed)
  0x005934C0  (unnamed)
  0x00593450  CExoArrayList<uint16>::Add
  0x0073F750  ClientApp::GetGuiInGame
  0x006A20F0  (unnamed)
struct offsets touched (top 5 of [REG + 0xNN], EBP/ESP excluded):
  +0x4 [dec 4]  x4  via EAX,EDX
  +0x38 [dec 56]  x2  via ECX,EDX
  +0x3A [dec 58]  x2  via ECX,EDX
  +0x8 [dec 8]  x1  via EDX
  +0x11C [dec 284]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 0089b100 FUN_0089b100 size=535 ret=4 callers=2 callees=5

void __thiscall FUN_0089b100(undefined4 param_1,int *param_2)

{
  bool bVar1;
  int iVar2;
  int local_24;
  uint local_20;
  uint local_1c;
  uint local_14;
  
  iVar2 = FUN_0073f750(param_1);
  if ((iVar2 + 0x103c == 0) || (*(int *)(iVar2 + 0x1040) < 1)) {
    for (local_14 = 0; (int)local_14 < (int)(uint)*(ushort *)(DAT_00a1b4d0 + 0x11c);
        local_14 = local_14 + 1) {
      iVar2 = FUN_006a20f0(local_14 & 0xffff);
      if ((((local_14 != 0xcf) && (local_14 != 0xee)) && (iVar2 != 0)) && (*(int *)(iVar2 + 8) != 0)
         ) {
        bVar1 = false;
        if (*(short *)(iVar2 + 0x38) == -1) {
          local_1c = 0;
        }
        else if (*(short *)(iVar2 + 0x3a) == -1) {
          local_1c = 1;
        }
        else {
          local_1c = 2;
        }
        for (local_24 = 0; local_24 < param_2[1 = byte +0x4]; local_24 = local_24 + 1) {
          iVar2 = FUN_006a20f0(*(undefined2 *)(*param_2 + local_24 * 2));
          if (iVar2 != 0) {
            if (*(short *)(iVar2 + 0x38) == -1) {
              local_20 = 0;
            }
            else if (*(short *)(iVar2 + 0x3a) == -1) {
              local_20 = 1;
            }
            else {
              local_20 = 2;
            }
            if (local_1c < local_20) {
              bVar1 = true;
            }
            if (bVar1) {
              FUN_005934c0(local_14 & 0xffff,local_24);
              break;
            }
          }
/* ... 14 more lines truncated by --max-lines */
```

---
## CARD C123
**CLAIM:** Function at 0x0089C350 is `CSWGuiLevelUpIconGrid::HandleNavKey`: arrow-key navigation over rows/columns, skipping empty (-1) slots.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0089C350, size 617, header ret=4 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x0089C350 (FUN_0089c350)
string literals (0):
direct callees (3):
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x0089A0E0  (unnamed)
  0x0073FEA0  CClientExoApp::GetSWGuiManager
struct offsets touched (top 4 of [REG + 0xNN], EBP/ESP excluded):
  +0xC [dec 12]  x17  via EAX,ECX,EDX
  +0xD [dec 13]  x15  via EAX,ECX,EDX
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x89C5D0 [dec 9029072]  x1  via EAX

DECOMPILE (offsets annotated; truncated):
// FN 0089c350 FUN_0089c350 size=617 ret=4 callers=3 callees=3

undefined4 __thiscall FUN_0089c350(int *param_1,undefined4 param_2)

{
  char cVar1;
  int iVar2;
  undefined4 uVar3;
  
  cVar1 = *(char *)((int)param_1 + 0xd);
  iVar2 = param_1[3 = byte +0xc];
  switch(param_2) {
  case 0x2f:
  case 0x3f:
    if ((char)param_1[3 = byte +0xc] != '\0') {
      uVar3 = 1;
      FUN_0073fea0(1);
      FUN_004122a0(uVar3);
      *(char *)(param_1 + 0x3 [dec 3] {int*: byte +0xc}) = (char)param_1[3 = byte +0xc] + -1;
    }
    break;
  case 0x30:
  case 0x40:
    if ((*(byte *)(param_1 + 0x3 [dec 3] {int*: byte +0xc}) < 2) &&
       (*(int *)(*(int *)(*param_1 + (uint)*(byte *)((int)param_1 + 0xd) * 4) + 0x108 +
                (uint)(byte)((char)param_1[3 = byte +0xc] + 1) * 0xb4) != -1)) {
      uVar3 = 1;
      FUN_0073fea0(1);
      FUN_004122a0(uVar3);
      *(char *)(param_1 + 0x3 [dec 3] {int*: byte +0xc}) = (char)param_1[3 = byte +0xc] + '\x01';
    }
    break;
  case 0x31:
  case 0x3d:
    if (*(char *)((int)param_1 + 0xd) == '\0') {
      *(char *)((int)param_1 + 0xd) = (char)param_1[1 = byte +0x4] + -1;
    }
    else {
      *(char *)((int)param_1 + 0xd) = *(char *)((int)param_1 + 0xd) + -1;
    }
    while (*(int *)(*(int *)(*param_1 + (uint)*(byte *)((int)param_1 + 0xd) * 4) + 0x108 +
                   (uint)*(byte *)(param_1 + 0x3 [dec 3] {int*: byte +0xc}) * 0xb4) == -1) {
      *(char *)(param_1 + 0x3 [dec 3] {int*: byte +0xc}) = (char)param_1[3 = byte +0xc] + -1;
    }
    break;
  case 0x32:
  case 0x3e:
    if ((uint)*(byte *)((int)param_1 + 0xd) == param_1[1 = byte +0x4] - 1U) {
      *(undefined1 *)((int)param_1 + 0xd) = 0;
/* ... 17 more lines truncated by --max-lines */
```

---
## CARD C124
**CLAIM:** Class `CSWGuiFeatsCharGen` has vftable 0x009AA714; its constructor is 0x0090C6A0.

```
ASM lines of ctor 0x0090C6A0 containing a 0x9AAxxx constant:
0090c6ea  c70114a79a00          MOV dword ptr [ECX],0x9aa714
0090cac9  6800a79a00            PUSH 0x9aa700
0090cb42  682ca39a00            PUSH 0x9aa32c
0090cb7e  6818a39a00            PUSH 0x9aa318
0090cbbb  68fca29a00            PUSH 0x9aa2fc
0090cbf8  68e0a29a00            PUSH 0x9aa2e0

ANCHORS of ctor:
anchors for 0x0090C6A0 (FUN_0090c6a0)
string literals (46):
  0x009AA700  at 0090cac9  'SKCHRGEN_p'
  0x009A969C  at 0090cae9  'MAIN_TITLE_LBL'
  0x009AA32C  at 0090cb42  'SUB_TITLE_LBL'
  0x009AA318  at 0090cb7e  'REMAINING_BOX_LBL'
  0x009AA2FC  at 0090cbbb  'REMAINING_SELECTIONS_LBL'
  0x009AA2E0  at 0090cbf8  'SELECTIONS_REMAINING_LBL'
  0x009AA6F4  at 0090cc34  'COST_LBL'
  0x009AA6E4  at 0090cc71  'COST_POINTS_LBL'
  0x009AA6D4  at 0090ccae  'CLASSSKL_LBL'
  0x009A1810  at 0090ccea  'LB_DESC'

VTABLE DUMP:
0x9AA714 (28 slots)
  slot 0 +0x0 = 0x90E410
  slot 15 +0x3C = 0x90E850
  slot 18 +0x48 = 0x90E810
```

---
## CARD C125
**CLAIM:** Function at 0x0090C230 is `CSWGuiFeatsCharGen::BuildChoosableFeatVector`: builds panel+0x1BC0 choosable feat vector, CanChooseFeat per id.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0090C230, size 344, header ret=0 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
ANCHORS (script):
anchors for 0x0090C230 (FUN_0090c230)
string literals (0):
direct callees (5):
  0x00733780  CExoString::~CExoString
  0x00593450  CExoArrayList<uint16>::Add
  0x00733540  CExoString::Constructor
  0x008480B0  CSWCLevelUpStats::CanChooseFeat
  0x0090C620  (unnamed)
struct offsets touched (top 6 of [REG + 0xNN], EBP/ESP excluded):
  +0x68 [dec 104]  x1  via EAX
  +0x118 [dec 280]  x1  via EDX
  +0x11E [dec 286]  x1  via ECX
  +0x310 [dec 784]  x1  via ECX
  +0x1BDC [dec 7132]  x1  via EAX
  +0x1BDD [dec 7133]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 0090c230 FUN_0090c230 size=344 ret=0 callers=3 callees=5

void __fastcall FUN_0090c230(int param_1)

{
  undefined2 uVar1;
  int iVar2;
  uint local_24;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00968348;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  for (local_24 = 0; (int)local_24 < (int)(uint)*(ushort *)(DAT_00a1b4d0 + 0x11e);
      local_24 = local_24 + 1) {
    uVar1 = *(undefined2 *)(*(int *)(DAT_00a1b4d0 + 0x118) + (local_24 & 0xffff) * 2);
    iVar2 = FUN_0090c620(uVar1);
    if (iVar2 == -1) {
      iVar2 = FUN_0090c620(uVar1);
      if (iVar2 == -1) {
        iVar2 = FUN_0090c620(uVar1);
        if (iVar2 == -1) {
          iVar2 = FUN_0090c620(uVar1);
          if (iVar2 == -1) {
            iVar2 = FUN_008480b0(uVar1,*(undefined1 *)(param_1 + 0x1bdc),
                                 *(undefined1 *)(param_1 + 0x1bdd),param_1 + 0x1bcc);
            if (iVar2 != 0) {
              FUN_00593450(uVar1);
            }
          }
        }
      }
    }
  }
  local_8 = 0xffffffff;
  FUN_00733780();
  ExceptionList = local_10;
  return;
}
```

---
## CARD C126
**CLAIM:** Hook site at 0x0089AB29 inside the function at 0x0089AAA0: bytes `89 4d b8 8b 15 d0 b4 a1 00`. At this point power grid builder loop after mov ecx,[eax]: ecx = spellId (uint); window EAX-free. Consumed-exit (skip) target 0x0089AEB8 = loop increment (mov eax,[ebp-0x10]).

```
SITE WINDOW (>> = claimed address):
   0089AB12  0f 8d ae 03 00 00  JGE 0x0089aec6
   0089AB18  8b 45 f0           MOV EAX,dword ptr [EBP + -0x10]
   0089AB1B  8b 4d d8           MOV ECX,dword ptr [EBP + -0x28]
   0089AB1E  8d 14 81           LEA EDX,[ECX + EAX*0x4]
   0089AB21  89 55 c0           MOV dword ptr [EBP + -0x40],EDX
   0089AB24  8b 45 c0           MOV EAX,dword ptr [EBP + -0x40]
   0089AB27  8b 08              MOV ECX,dword ptr [EAX]
>> 0089AB29  89 4d b8           MOV dword ptr [EBP + -0x48],ECX
   0089AB2C  8b 15 d0 b4 a1 00  MOV EDX,dword ptr [0x00a1b4d0]
   0089AB32  89 55 bc           MOV dword ptr [EBP + -0x44],EDX
   0089AB35  8b 45 b8           MOV EAX,dword ptr [EBP + -0x48]
   0089AB38  50                 PUSH EAX
   0089AB39  8b 4d bc           MOV ECX,dword ptr [EBP + -0x44]
   0089AB3C  8b 89 04 01 00 00  MOV ECX,dword ptr [ECX + 0x104]
   0089AB42  e8 09 69 e2 ff     CALL 0x006c1450

EXIT WINDOW:
   0089AEAF  52                 PUSH EDX
   0089AEB0  8b 4d 84           MOV ECX,dword ptr [EBP + -0x7c]
   0089AEB3  e8 78 00 00 00     CALL 0x0089af30
>> 0089AEB8  8b 45 f0           MOV EAX,dword ptr [EBP + -0x10]
   0089AEBB  83 c0 01           ADD EAX,0x1
   0089AEBE  89 45 f0           MOV dword ptr [EBP + -0x10],EAX
   0089AEC1  e9 36 fc ff ff     JMP 0x0089aafc
```

---
## CARD C127
**CLAIM:** Function at 0x0089B100 is `CSWGuiLevelUpIconGrid::BuildSortedPowerList`: sorts spells.2da rows (type +0x14C in {1,6}, name strref != 0) by multi-digit key, caches at +0x1048.

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0089B100, size 535, header ret=4 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x0089B100 (FUN_0089b100)
string literals (0):
direct callees (5):
  0x0089CD50  (unnamed)
  0x005934C0  (unnamed)
  0x00593450  CExoArrayList<uint16>::Add
  0x0073F750  ClientApp::GetGuiInGame
  0x006A20F0  (unnamed)
struct offsets touched (top 5 of [REG + 0xNN], EBP/ESP excluded):
  +0x4 [dec 4]  x4  via EAX,EDX
  +0x38 [dec 56]  x2  via ECX,EDX
  +0x3A [dec 58]  x2  via ECX,EDX
  +0x8 [dec 8]  x1  via EDX
  +0x11C [dec 284]  x1  via ECX

DECOMPILE (offsets annotated; truncated):
// FN 0089b100 FUN_0089b100 size=535 ret=4 callers=2 callees=5

void __thiscall FUN_0089b100(undefined4 param_1,int *param_2)

{
  bool bVar1;
  int iVar2;
  int local_24;
  uint local_20;
  uint local_1c;
  uint local_14;
  
  iVar2 = FUN_0073f750(param_1);
  if ((iVar2 + 0x103c == 0) || (*(int *)(iVar2 + 0x1040) < 1)) {
    for (local_14 = 0; (int)local_14 < (int)(uint)*(ushort *)(DAT_00a1b4d0 + 0x11c);
        local_14 = local_14 + 1) {
      iVar2 = FUN_006a20f0(local_14 & 0xffff);
      if ((((local_14 != 0xcf) && (local_14 != 0xee)) && (iVar2 != 0)) && (*(int *)(iVar2 + 8) != 0)
         ) {
        bVar1 = false;
        if (*(short *)(iVar2 + 0x38) == -1) {
          local_1c = 0;
        }
        else if (*(short *)(iVar2 + 0x3a) == -1) {
          local_1c = 1;
        }
        else {
          local_1c = 2;
        }
        for (local_24 = 0; local_24 < param_2[1 = byte +0x4]; local_24 = local_24 + 1) {
          iVar2 = FUN_006a20f0(*(undefined2 *)(*param_2 + local_24 * 2));
          if (iVar2 != 0) {
            if (*(short *)(iVar2 + 0x38) == -1) {
              local_20 = 0;
            }
            else if (*(short *)(iVar2 + 0x3a) == -1) {
              local_20 = 1;
            }
            else {
              local_20 = 2;
            }
            if (local_1c < local_20) {
              bVar1 = true;
            }
            if (bVar1) {
              FUN_005934c0(local_14 & 0xffff,local_24);
              break;
            }
          }
/* ... 14 more lines truncated by --max-lines */
```

---
## CARD C128
**CLAIM:** Function at 0x0090A970 is `CSWGuiFeatsCharGen::Constructor`: feats level-up panel ctor (size 0x1C08, grid ptr at +0x1BF4).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0090A970, size 44, header ret=4 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x0090A970 (FUN_0090a970)
string literals (0):
direct callees (2):
  0x009196FD  operator delete
  0x0090A9A0  (unnamed)
struct offsets touched (top 0 of [REG + 0xNN], EBP/ESP excluded):

DECOMPILE (offsets annotated; truncated):
// FN 0090a970 FUN_0090a970 size=44 ret=4 callers=0 callees=2

undefined4 __thiscall FUN_0090a970(undefined4 param_1,uint param_2)

{
  FUN_0090a9a0();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

---
## CARD C129
**CLAIM:** Function at 0x0090E410 is `CSWGuiSkillsCharGen::Constructor`: skills level-up panel ctor (8 fixed skill rows).

```
TIER-0 FACTS (script-computed, trust these):
- entry 0x0090E410, size 44, header ret=4 (0 = no stack args popped / fastcall-or-thiscall w/o stack args)
- 16-byte aligned: YES
- standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
ANCHORS (script):
anchors for 0x0090E410 (FUN_0090e410)
string literals (0):
direct callees (2):
  0x0090E440  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0 of [REG + 0xNN], EBP/ESP excluded):

DECOMPILE (offsets annotated; truncated):
// FN 0090e410 FUN_0090e410 size=44 ret=4 callers=0 callees=2

undefined4 __thiscall FUN_0090e410(undefined4 param_1,uint param_2)

{
  FUN_0090e440();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```
