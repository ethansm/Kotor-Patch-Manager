# Opus gate B0, part C: sample of 102 reviewer matches and 8 AUTO meaning checks

Gate: Opus, 2026-10-03, following `OPUS_GATE_BRIEF.md`. Verdict rows are in `overrides_c.tsv`.

Method:
- For every row, compared the GOG `fp.tsv` and Steam `fp.tsv` (size, retn, n_insn, callee count, disp, imm).
- Checked the run delta against nearby trusted rows (`B0/final_rows.tsv` plus `steam_db_pr3/verified.tsv`).
- Checked callee identity: either a trusted mapping, the local run delta, or the callee fingerprint being equal to the GOG callee's fingerprint.
- Looked for same-key twins within 4 starts and settled each twin by content.
- Compared shape and callee names against the Mac x86 symbol of the same name.
- Read the Steam decompile or asm for meaning.

Overall:
- 106 of 110 rows have a GOG fingerprint equal to the Steam fingerprint.
- The other 4 differ only by a Ghidra function split or a 1-byte or empty-string difference (see the notes on ExecuteCode, RunScript, ChildRemove and GetIcon).
- Every candidate is the correct body counterpart: no `rejected` rows and no corrections.
- Two rows have GOG names that contradict their bodies (`label_suspect`).

Verdict counts:

| verdict | all 110 | sample 102 | AUTO 8 |
|---|---|---|---|
| verified | 75 | 68 | 7 |
| likely | 33 | 33 | 0 |
| label_suspect | 2 | 1 | 1 |

`likely` means a getter, setter or stub whose only anchors are the field offset (with Mac disp equal), the fingerprint and the run delta.

## Sample: rows that needed more than fingerprint equality

Twins with an identical fingerprint within 4 starts were all settled by content:

- **StackPush/Pop wrappers (CVirtualMachine)**: each thunk calls a different, trusted VMI callee, for example `006fd9d1 CALL 0x00702be0` for VMI::StackPushInteger.
- **CServerExoApp LoadGame / SaveGame**: the callees are the trusted Internal::LoadGame 0x52fb80 and Internal::SaveGame 0x52f610.
- **CClientExoApp 22-byte wrappers** (GetPlayerCreature, ExitProgram, GetActivePauseState, SetEventDescriptions, CancelMovie, StopCreditSequence, SetCombatMode, DisplayMainMenu, SetVideoMode, GetPausedByCombat): in each case the Steam callee's fingerprint equals the GOG CClientExoAppInternal callee, and none of the twins' callees match.
- **EnableInput**: twin 0x7402d0 also matched by callee fingerprint.
  - 0x7402f0 calls 0x7b3390, which writes `[+0x2bc]=0`.
  - 0x7402d0 calls 0x7b3370, which writes 1. That is DisableInput, which comes first in the GOG order.
  - Mac Internal Enable/DisableInput both use disp 0x2bc.
- **SendSave/SendLoadGameRequest**: both chains end in a message sender.
  - `0087a494 PUSH 0x4` (Save) and `0087a514 PUSH 0x5` (Load).
  - Mac CSWCMessage::SendPlayerToServerModule_SaveGame / _LoadGame use imm 0x4 / 0x5.
- **CWorldTimer GetWorldTimeSecond / GetWorldTimeMillisecond**: four 49-byte twins call the same split function 0x51ab40.
  - In 0x51ab40, `*param_6 = t % 1000` and `*param_5 = (t/1000) % 60`.
  - 0x51b180 returns param_5 (seconds) and 0x51b1c0 returns param_6 (milliseconds).
- **CResGFF ReadFieldCHAR / WriteFieldCHAR**:
  - Read: `00718ad2 CMP dword ptr [EDX],0x1`. The BYTE twin compares 0.
  - Write: `00719a6d PUSH 0x1` and `MOVSX`. The BYTE twin uses `PUSH 0x0` and `MOVZX`.
- **Scene Enable/DisableAnimations**: `004651ba MOV byte [EAX+0xe6],0x1` and `004651da ... ,0x0`.
- **ItemPropertyHandler RemoveRegeneration / RemoveBlasterBoltDeflectionIncrease**: three 233-byte twins differ only in the compared global:
  - `005b23b3 CMP EDX,[0x00995f38]` = 7;
  - `005b25d3 [0x00996088]` = 92;
  - 0x5b2780 `[0x0099608c]` = 93.
  - 7 is the regenerate effect type in NWN numbering. The same numbering gives 15 = temporary HP below, so Regeneration is `verified`.
  - Which of 92 and 93 is Increase rests on GOG order only, so that row is `likely`.
- **ExecuteCommandEffectTemporaryForcePoints**: `00676f05 MOV EAX,[0x00996084]` = 91, a K2-only type. Twin 0x677010 uses `[0x00995f58]` = 15 (temporary hitpoints), which is the GOG TemporaryHP at 0x78c5f0.
- **CSWCObject::AnimationParry**: `007ef847 MOV EAX,[0x00a0fe3c]`. That handle is stored at `006eb9f6` from `006eb9c1 PUSH 0x99af74` "Parry". Twins use 0xa0fe4c and 0xa0fe50.
- **GetBaseWillSavingThrow**: `006b7675 CALL 0x006f0990`. The GOG-to-Steam delta is 0x1e1950 for all three save-bonus callees: Fort 0x50efd0->0x6f0920, Reflex 0x50f000->0x6f0950, Will 0x50f040->0x6f0990.
- **GivePlotXP, RemoveGameInProgress, ConsoleFunc::Constructor, CRes::Request / CancelRequest**: their twins' callees differ (the candidate's are trusted or match by delta).

Rows where the fingerprints differ:

- **CVirtualMachineInternal::ExecuteCode (0x642c30)**: Ghidra split the GOG function. The GOG span 0x642c30..0x6450aa is 9338 B, against Steam's 9336 B. The delta 0xbcdc0 is the same as DeleteScript's. Steam calls 5 trusted VMI StackPush* functions.
- **CVirtualMachineInternal::RunScript (0x645540)**: the GOG side is a 9-byte head plus 0x645549 (731 B), total 740 B, equal to Steam. Steam calls 0x701fd0 (= GOG ReadScriptFile 0x645270 + 0xbcd60) and 0x702850 (= GOG 0x645af0 + 0xbcd60).
- **Scene::ChildRemove**: 111 B vs 112 B, otherwise identical. Its callee UnlistSpecialParts maps by delta.

Other meaning checks:
- Dawn/Dusk:
  - Dusk 0x7a92c0 loads the float constant `DAT_0098d930` and sets `[+0xa4]=0`; Mac Dusk has imm 0x3d4ccccd.
  - Dawn 0x7a9360 stores 0 and sets `[+0xa4]=1`; Mac Dawn has no float imm.
- ShowShadowBlob 0x764f80 makes a vt+0xac call; Mac Show uses vt+0xb0, the usual Mac +4 offset.
- The real HideShadowBlob is 0x764fb0 (vt+0xb0, Mac 0xb4).

### Sample error (1)

**GOG 0x7c1880 `CSWSItemPropertyHandler::CSWSItemPropertyHandler` -> Steam 0x5aa470: the counterpart is right, but the name contradicts the body.** The reviewer answered `match` with confidence 3 and noted "only installs base vftable".
- The whole body is `005aa47a MOV dword ptr [EAX],0x99247c`. In Ghidra's RTTI naming, 0x99247c is `CItemPropertyApplierRemover::vftable`.
- The CSWSItemPropertyHandler vptr is 0x992468:
  - the inlined construction in 0x51d2f0 stores the base vptr first (`0051d3c3 MOV [ECX],0x99247c`), then the derived one (`0051d3cc MOV [EDX],0x992468`);
  - `~CSWSItemPropertyHandler` 0x5aa3c0 starts with `005aa3eb MOV [EAX],0x992468` and ends by restoring the base vptr 0x99247c.
- The only caller is an EH unwind funclet: Steam `00952bb3 JMP 0x005aa470`, and on GOG 0x7c1880 is called only from `Unwind@0097a380`.
- Unwind code runs destructors, not constructors. The body therefore fits `CItemPropertyApplierRemover::~CItemPropertyApplierRemover` (base dtor, which resets the vptr). There is no Mac symbol to confirm it.
- Verdict: `label_suspect`.

All other 101 sampled matches are correct.

**sample_errors: 1** (any sample error escalates B0 to a 100% Opus read, per DESIGN 4.6).

## AUTO meaning checks

| route | GOG -> Steam | name | result | anchor |
|---|---|---|---|---|
| sig | 0x746300 -> 0x5a3a10 | CSWSEffectListHandler::OnApplyLimitMovementSpeed | PASS | `005a3a3d MOV EDX,[ECX+0xfc]` test, then `005a3a66 MOV [ECX+0xff0],0x1` on the vt+0x30 creature. Mac disp 0x34 (vt+4),0xfc,0xff0. OnRemove is a separate function (GOG 0x746370). |
| sig | 0x70cd60 -> 0x552b40 | CSWSArea::GridDFSTransTableGet | **FAIL (meaning)** | Counterpart right (fp equal), name wrong. Details below. |
| sig | 0x6f4be0 -> 0x6b8760 | CSWSCreatureStats::RemoveKnownSpell | PASS | `006b877e CALL 0x006c1450` (GetSpell) then `006b87a5 CALL 0x00845a40` (RemoveKnownSpell). Both callees are fp-equal to the GOG ones; Mac has the same callees and disp 0x90,0x104. |
| sig | 0x584710 -> 0x8f0d30 | CSWCAreaOfEffectObject::ResetTimer | PASS | `008f0d6e ADD EDX,[ECX+EAX*4+0x114]` sums 3 duration slots, then writes `[[+0xfc]+i*4] = [+0x108+k*4]`. Mac disp 0x68,0xfc,0x108,0x114. |
| sig | 0x474800 -> 0x7ef6a0 | CSWCObject::AnimationOverlay | PASS | `007ef6f9 MOV EAX,[0x00a0fe20]`. That handle is stored at `006eb9ad` from `006eb978 PUSH 0x99af7c` "Overlay", then passed to the 2DA int getter `007ef707 CALL 0x0071b8b0`. |
| c2 | 0x765620 -> 0x61a210 | CSWSDoor::LoadDoor | PASS | `0061a25f PUSH 0x9958c8` "Appearance" and `0061b3c8 PUSH 0x996b80` "OnFailToOpen". Callees GetDetectDC, GetDisarmDC and SetOpenState are trusted. |
| c2 | 0x6794f0 -> 0x615cc0 | CSWSItem::GetIcon | PASS | `00615d00 MOV CL,[EAX+0x2a6]` / `00615d2a [EDX+0x2a4]` with `PUSH 0x6d`, then `00615d1b CALL 0x006d4e60` (GetIconResRef). Mac has the same disp, imm and callees. The GOG str 0x9858f5 is an empty string. |
| c2 | 0x486470 -> 0x6ed4d0 | CTwoDimArrays::Load2DArrays_AlienVO | PASS | `006ed51c PUSH 0x99b140` "AlienVO" -> CResRef 0x710840 -> C2DA 0x71ad40 -> Load2DArray 0x71c3d0, stored at `[+0x14c]`. |

### AUTO failure: CSWSArea::GridDFSTransTableGet (pipeline-rule failure)

**What the body is.** The sig route accepted GOG 0x70cd60 -> Steam 0x552b40 on fingerprint equality plus `run_ok`, and the body pairing is correct. The meaning is wrong:
- Steam 0x552b40 is a 48-B hash helper: `00552b53 LEA EDX,[EAX+ECX*1+0x1770]` after `IMUL x,0x11` / `IMUL y,0x17`, then `AND EAX,0x800000ff`, which is a signed mod 256.
- It stores ECX but never reads `this`.
- It has 4 callers: 0x552b70, 0x552c90, 0x555dd0 and 0x555f10. That means it serves both the GridDFS and the PathPointDFS tables.

**What Get really is.**
- Mac has no separate hash function. Mac `GridDFSTransTableGet` (0x429000, 172 B) inlines the same constants (0x11, 0x17, 0x1770, 0xffffff00) and walks the table at `[this+0x1b8]`.
- That matches Steam **0x552b70** (273 B, RET 0x10): it calls the hash and then makes two probes of `[this+0x1b8]` (`iVar1 + 0x100`).
- GOG 0x70cd90, the counterpart of 0x552b70, is labelled `PlotSoundPath`. Mac `PlotSoundPath` is a 10-B stub.
- The GOG name therefore belongs at GOG 0x70cd90 -> Steam 0x552b70.

**A label rotation in this family**, in GOG DB order:

| GOG | GOG label | body is | Steam | B0 status |
|---|---|---|---|---|
| 0x709770 | GridDFSTransTableInitialize | path smoothing (reviewer) | 0x54f3d0 | label_suspect |
| 0x70ccb0 | GridDFSTransTablePut | Initialize: `PUSH 0x2000` alloc, matches Mac Initialize shape (disp 0x1b8 x6, imm 0x80000000); RE_AGENT_GUIDE 3.5 | 0x552a90 | **AUTO verified (wrong name, not in this part)** |
| 0x70cd60 | GridDFSTransTableGet | hash helper (no Mac symbol) | 0x552b40 | label_suspect (this check) |
| 0x70cd90 | PlotSoundPath | GridDFSTransTableGet | 0x552b70 | label_suspect (reviewer) |
| 0x70ceb0 | PathPointDFSTransTableInitialize | GridDFSTransTablePut: RET 0x10, calls hash, Mac Put shape | 0x552c90 | label_suspect (reviewer) |

**Why the rule failed.** The sig route checks only body equality, so it cannot see GOG label errors. Here the GOG label sits on a helper that Mac inlined.

**Suggested rule.** Demote an AUTO row to REVIEW when either of these holds:
- the same-name Mac body is outside 0.5x-2x the GOG size and carries a member disp that the GOG side lacks (here 0x1b8);
- or the GOG body never reads ECX while the name is a member method.

The B0 row `GridDFSTransTablePut 0x70ccb0 -> 0x552a90 verified` should be overridden by whoever owns AUTO rows. Its Steam row should ship as `GridDFSTransTableInitialize = 0x552a90` only with an Opus override.

## Side findings outside part C (for the lead)

- **HideShadowBlob (100% list, GOG 0x410c80)**: Steam 0x764fb0 is the body. It is FID-labelled `CPaneContainerManager::NotifyPaneDivider` and calls `[+0x68]` vt+0xb0 (Mac Hide vt+0xb4). The reviewer's corrected address 0x764fb0 is right. The candidate 0x78bc50 is a CGuiInGame teardown.
- **CExoString::Constructor_2 (GOG 0x605680) -> 0x7338d0 and CExoString::Destructor (0x605890) -> 0x733540 are wrong.**
  - 0x733570 (126 B) is the char* ctor: `006eb9c1 PUSH 'Parry'; CALL 0x00733570`.
  - 0x733780 (60 B) is the dtor.
  - The Steam GetBaseWillSavingThrow and RemoveGameInProgress call these, matching their GOG callees.
  - The reviewer corrections already say so; both rows are `unresolved` in final_rows.
