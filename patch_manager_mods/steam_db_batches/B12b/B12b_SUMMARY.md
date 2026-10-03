# B12b summary: CSWSCreature, GOG >= 0x696F00 (2026-10-03)

Scope: 201 GOG rows, all class `CSWSCreature`, from GOG 0x696F00 (the B12a/B12b split) to 0x6E0170, plus one row
(0x6F5300 GetRelativeWeaponSize) that sits in the CSWSCreatureStats block.

Inputs:
- `evidence.tsv`. Gates: AUTO 149 (route c2 97, sig 52), REVIEW 32, REJECT 17, NOCAND 3.
- `final_rows.tsv` pre-Opus finalize at 2cad19d (counts below). The working file is the b7114f4 finalize with parts a+r.
- `review/sonnet_c1.tsv`: one Sonnet chunk, 43 cards (33 real, 10 decoys: 5 prev / 5 next). Decoys accepted 0.
- `opus_gate.md/json`: **ESCALATE**, because Opus part a found 1 sample error (a rotated GOG label). So 100% of the
  reviewer matches got an Opus read.
- The Opus parts (disjoint, 63 GOG addresses, no conflict):
  - `opus/overrides_a.tsv/.md`: 27 rows (verified 18, label_suspect 9), incl. the 8 AUTO meaning checks (all pass);
    sample_errors 1.
  - `opus/overrides_r.tsv/.md`: 20 REJECT/NOCAND rows (verified 15, likely 2, label_suspect 3); no sample rows.
  - `opus/overrides_z.tsv/.md`: 16 rows = 14 part rows (5 mac_rotation 100% rows + the 9 remaining sample rows) and
    2 rows from the AUTO rotation audit (verified 8, label_suspect 8); sample_errors 2.
- `overrides.tsv` holds a+r (47 rows). **Part z is not merged yet** (open item 1).

This consolidation writes: `opus/overrides_z.tsv/.md`, this file, a `## Lessons` section in `gate_report.md`, and a
`## B12b` section in `../GOG_LABEL_SUSPECTS.md`. Nothing was run; no DB was written.

## Pre-Opus finalize (for reference)

| verdict | rows |
|---|---|
| verified | 168 (AUTO 149 + REVIEW 19) |
| likely | 1 (HasAction) |
| label_suspect | 7 (reviewer label_suspect) |
| unresolved | 8 (REVIEW 5 reviewer corrections + NOCAND 3) |
| rejected | 17 (REJECT) |
| **total** | **201** |

## Sample errors

All 3 are GOG label errors on a correct Steam counterpart; no sampled reviewer `match` had a wrong address.
- part a: 0x6C66E0 AIActionCheckInterAreaPathfinding (body = MoveAwayFromLocation, the rotation).
- part z: 0x69A160 AddChemical and 0x69A240 AddComponent (names swapped; the reviewer wrote "label may be swapped" but
  still ruled `match`).

Not counted as sample (100% rows), but also reviewer `match` on a wrong name: 0x6C6230, 0x6C6930, 0x6C7820, 0x6DE780.
The reviewer's address calls were right on every real card; its name calls failed on every rotated row.

## The 27 address corrections (final Steam address != gate pick)

Delta context: the gate used the batch run delta -0x118B30, which is right only for GOG 0x696F00-0x6A5000 (SetInParty
is -0x118AE0). From GOG 0x6C2820 the true delta is -0x10ADC0, from about 0x6DB820 -0x10AC20 (-0x10AC40 for
Appear/Disappear), and 0x6F5300 sits in the Stats block at -0x3C480. Every pick below that came from the run delta
landed on unrelated code (CRT, GUI, SEH funclets at 0x98xxxx).

| GOG | name | gate pick | final | verdict | part |
|---|---|---|---|---|---|
| 0x6A2E10 | SetInParty | 0x58A610 (GetSpikeDetails) | 0x58A330 | verified | a |
| 0x6A4D30 | UseLoreOnItem | 0x58C250 | 0x58C290 | label_suspect | a |
| 0x6D5A90 | WUL_QW_FL_OffsetWaitDest | 0x412C00 | 0x5CACD0 | verified | a |
| 0x6D5BC0 | WUL_QW_FL_PathWaitDest | 0x66B640 | 0x5CAE00 | verified | a |
| 0x6D5CF0 | WUL_QW_FL_ToOffsetWaitDest | 0x66B770 | 0x5CAF30 | verified | a |
| 0x6D5E20 | WUL_QW_FL_ToPathWaitDest | 0x66B8A0 | 0x5CB060 | verified | a |
| 0x6D5F50 | WUL_QW_FL_TeleportForward | 0x5BD3E0 | 0x5CB190 | verified | a |
| 0x6C2820 | SetDetectMode | none (NOCAND) | 0x5B7A60 | verified | r |
| 0x6C7F90 | CanEquipWeapon | 0x405100 | 0x5BD1D0 | verified | r |
| 0x6C81A0 | CanEquipWeapon2 | 0x670C90 | 0x5BD3E0 | verified | r |
| 0x6C8800 | CheckProficiencies | 0x405970 | 0x5BDA40 | verified | r |
| 0x6C9D80 | AIActionCheckMoveAwayFromObject | none (NOCAND) | 0x5BEFC0 | label_suspect (= AreaWait) | r |
| 0x6CA5B0 | AIActionPartyFollowLeader | 0x971DC0 (SEH funclet) | 0x5BF7F0 | verified | r |
| 0x6D2130 | WUL_QW_FL_FindPath | 0x67AC20 | 0x5C7370 | verified | r |
| 0x6D35A0 | WUL_QW_FL_Moving_To_Path | 0x669020 | 0x5C87E0 | verified | r |
| 0x6D4DC0 | WUL_QW_FL_Moving_To_Offset | 0x5BBF40 (MoveToObject) | 0x5CA000 | verified | r |
| 0x6D5460 | WUL_QW_FL_WaitingForPath | 0x67DF50 | 0x5CA6A0 | verified | r |
| 0x6DB820 | AIActionWaitForEndOfRound | 0x418990 | 0x5D0C00 | verified | r |
| 0x6DBB50 | UpdateVisibleList | 0x5C3270 | 0x5D0F30 | verified | r |
| 0x6DC3F0 | AIActionRest | 0x983C00 | 0x5D17D0 | likely (stub, case 0x2A) | r |
| 0x6DC410 | AIActionTaunt | 0x983C20 | 0x5D1990 | likely (ICF fold, see below) | r |
| 0x6DC430 | AIActionAppear | 0x983C40 | 0x5D17F0 | verified | r |
| 0x6DC500 | AIActionDisappear | 0x983D10 | 0x5D18C0 | verified | r |
| 0x6DD5D0 | AIActionSurrenderToEnemies | 0x984DE0 | 0x5D29B0 | verified | r |
| 0x6DDAC0 | AIActionStanceMeditative | 0x9852D0 | 0x5D2EA0 | verified | r |
| 0x6DEE60 | AIActionAnimalEmpathy | 0x41BFD0 | 0x5D4240 | label_suspect (false stub) | r |
| 0x6F5300 | GetRelativeWeaponSize | none (NOCAND) | 0x6B8E80 | label_suspect (= Stats::GetWeaponFocus) | r |

Relative to the pre-Opus finalize only 21 Steam addresses change: the reviewer corrections for the 5 QuickWalk rows
and UseLoreOnItem were already in `final_rows.tsv` (recheck re-gate) but left unresolved / label_suspect.

**ICF fold:** Steam 0x5D1990 is one 18 B body that RunActions uses for cases 0x29 (AnimalEmpathy), 0x2B (Taunt) and
0x36 (PickPocket). A hook on `AIActionTaunt` fires for all three actions.

## Label-rotation findings (GOG DB names; the Steam counterparts are all fp 1:1)

1. **AIAction cluster GOG 0x6C6230-0x6C7820 (Steam 0x5BB470-0x5BCA60).** The bodies follow the Mac x86 symbol order
   exactly; 5 GOG labels sit on the wrong slot and the MoveAwayFromObject label is 0x2A00 away:

   | GOG | GOG label | body (Mac symbol) | Steam | name really at (GOG -> Steam) |
   |---|---|---|---|---|
   | 0x6C6230 | MoveAwayFromLocation | ChangeFacingPoint | 0x5BB470 | 0x6C66E0 -> 0x5BB920 |
   | 0x6C6360 | MoveToPointRadius | CheckMoveAwayFromObject | 0x5BB5A0 | 0x6C7820 -> 0x5BCA60 |
   | 0x6C66E0 | InterAreaPathfinding | CheckMoveAwayFromLocation | 0x5BB920 | 0x6C6930 -> 0x5BBB70 |
   | 0x6C6930 | ChangeFacingPoint | CheckInterAreaPathfinding | 0x5BBB70 | 0x6C6230 -> 0x5BB470 |
   | 0x6C7820 | CheckMoveToPoint_2 | CheckMoveToPointRadius | 0x5BCA60 | MoveToPoint = 0x6C74C0 -> 0x5BC700 (AUTO, correct) |
   | 0x6C9D80 | CheckMoveAwayFromObject | AIActionAreaWait | 0x5BEFC0 | 0x6C6360 -> 0x5BB5A0 |

2. **CanUseItem restriction block GOG 0x6DDEE0-0x6DEE60.** Class sits on the RemoveFromInvitationsOffered label, Gender
   (no GOG label) on the Class label, and the AnimalEmpathy label is on a 21 B false stub that the Class check calls.
   The real RemoveFromInvitationsOffered is probably GOG 0x6DF1B0 (FUN_) -> Steam 0x5D4590 (shape only).
3. **AddChemical / AddComponent swap** (GOG 0x69A160 / 0x69A240): feedback ids 0xE9/0xEA and the Get/Set callees decide.
   The GOG Get/SetComponent and Get/SetChemical labels themselves are right.
4. **Isolated label errors (part a/r):** 0x698DF0 (= GetDetectMode), 0x69DF00 (= GetBlind), 0x6A2600 (inlined
   UpdateActionQueue helper), 0x6A26D0 (= RemoveSpellActionFromRound), 0x6A3E00 (= SetLastInstantEquip), 0x6A4D30 /
   0x6A4DA0 (= CExoArrayList<float>::Add / Insert), 0x6DC300 (visible-list contains), 0x6F5300 (= Stats::GetWeaponFocus).
5. Below the batch (B12a range, not B12b rows): GOG labels GetBlind 0x68F7D0 (= CancelAction, confirmed in part z),
   GetDetectMode 0x693490, SetLastInstantEquip 0x696DB0, AddPickUpItemActions 0x693AC0 (likely ComputeArmourClass).

All 20 label_suspect rows are recorded in `../GOG_LABEL_SUSPECTS.md` section `## B12b`.

## AUTO rotation audit (part z, job 2)

14 AUTO rows inside the two rotated windows (GOG 0x6C6000-0x6C7A00, 0x6DDE00-0x6DF000) were checked against the Mac
symbol of the same name. 12 pass, **2 fail** and get `label_suspect` overrides: 0x6C6360 (MoveToPointRadius label,
MoveAwayFromObject body) and 0x6DDEE0 (RemoveFromInvitationsOffered label, CheckItemClassRestrictions body). Together
with the 8 AUTO meaning checks of part a (all pass; 0x6DE0A0 is in both sets), 21 distinct AUTO rows of 149 had an
Opus meaning read; 2 were wrong, both inside a rotation cluster.

## Expected final verdicts after `finalize B12b --opus-sample-errors 3`

Computed by applying overrides a + r + z (the verdict and steam_addr of each) to `final_rows.tsv`:

| verdict | rows |
|---|---|
| verified | **179** |
| likely | 2 (AIActionRest, AIActionTaunt) |
| label_suspect | 20 |
| rejected | 0 |
| unresolved | 0 |
| **total** | **201** |

- Moves from pre-Opus: rejected -> verified 14; verified -> label_suspect 10; unresolved -> verified 6; unresolved ->
  label_suspect 2; rejected -> likely 2; rejected -> label_suspect 1; likely -> verified 1 (HasAction).
- Tiers of shipped verified rows: opus 41, AUTO 138 (149 AUTO minus 11 with an Opus override).
- `functions.csv` should go from 187 rows (current, a+r) to **179**: the 8 part-z label_suspect rows drop out.
- **Duplicate check:** no Steam address is held by two names among the verified / likely / label_suspect rows. The
  Taunt row ships 0x5D1990 alone (AnimalEmpathy's GOG row is the 0x5D4240 stub; PickPocket has no B12b row).
- **Coverage check:** all 52 non-AUTO rows (REVIEW 32, REJECT 17, NOCAND 3) have an Opus override; 0 uncovered.
- Every override is fp 1:1 GOG vs Steam (size, retn, n_insn, disp, imm).

## Open items

1. Merge `opus/overrides_z.tsv` (16 rows, tag `[z]`) into `B12b/overrides.tsv` (47 -> 63 rows), then run
   `finalize B12b --opus-sample-errors 3` (a 1 + z 2) and check the counts above. Expect decoys_accepted 0.
2. `persist B12b --apply` (adds the `## Lessons` lines), `import-ghidra --mode append`, `SqliteTools validate`,
   `check-validated`.
3. Semantic extras (not shipped; the GOG rows carry the wrong names): ChangeFacingPoint 0x5BB470, MoveAwayFromObject
   0x5BB5A0, MoveAwayFromLocation 0x5BB920, InterAreaPathfinding 0x5BBB70, MoveToPointRadius 0x5BCA60, AreaWait
   0x5BEFC0, CheckItemClassRestrictions 0x5D32C0, CheckItemGenderRestrictions 0x5D3B60, AddComponent 0x581630,
   AddChemical 0x581710, CancelAction 0x576CA0. Decide whether to ship them by meaning (B0 `semantic:` precedent).
4. To locate: real UpdatePureGoodEvilPowers (likely GOG 0x6A42D0), ComputeArmourClass (likely GOG 0x693AC0), HasEffect,
   RemoveFromInvitationsOffered (likely GOG 0x6DF1B0 -> 0x5D4590, shape only).
5. KPM / mod impact: any hook on `AIActionTaunt` (Steam 0x5D1990) also fires for AnimalEmpathy and PickPocket; any
   GOG-side call by the rotated names above runs a different action handler.
