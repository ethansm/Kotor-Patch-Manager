# B10a summary: CSWSObject, CSWCObject (2026-10-03)

Scope: 245 GOG rows, CSWCObject 101 and CSWSObject 144. CSWCObject: GOG 0x410c30 run, 0x42b680, the client block
0x4725e0-0x477820 (delta +0x37aea0), and 0x6a2c90. CSWSObject: the 0x6637e0 run (delta -0x1248a0) and 0x6a5ce0-0x6bd870.

Inputs:
- `evidence.tsv`: AUTO 161 (c2 105, sig 56), REVIEW 70, REJECT 7, NOCAND 7.
- `final_rows.tsv` (pre-Opus finalize, counts below).
- `review/sonnet_c1.tsv` + `sonnet_c2.tsv`: 2 Sonnet chunks, 20 decoys (10 per chunk), 0 accepted, 0 unanswered.
- `opus_gate.md/json`: **OK** (20% sample, min 5). 9 rows at 100%, 13 sample rows, 8 AUTO meaning checks.
- The Opus parts (disjoint, 46 GOG addresses, no conflict):
  - `opus/overrides_a.tsv/.md`: 30 rows (verified 25, likely 4, label_suspect 1). The 8 AUTO meaning checks all pass;
    sample_errors 0.
  - `opus/overrides_r.tsv/.md`: 14 REJECT/NOCAND rows (verified 8, likely 3, label_suspect 3); no sample rows.
  - `opus/overrides_z.tsv/.md`: 2 rows, not an escalation (sample_errors 0). One is the AUTO rotation audit flag
    (0x6ae890, label_suspect); the other is the Destructor_3 duplicate (0x4727b0, rejected).
- `kdb_overrides.tsv`: 6 rows (3 demote, 3 rename).

This consolidation writes: the 3 override pairs, `kdb_overrides.tsv`, this file, `## Lessons` in `gate_report.md`, and
`gog_ledger_section.md`. Nothing was run, no DB was written, and `../GOG_LABEL_SUSPECTS.md` is untouched.

## Pre-Opus finalize (for reference)

| verdict | rows |
|---|---|
| verified | 190 (AUTO 161 + REVIEW 29) |
| likely | 35 (REVIEW) |
| label_suspect | 6 (the 5 Animation* rows + RemoveObjectFromDialog) |
| unresolved | 7 (NOCAND) |
| rejected | 7 (REJECT) |
| **total** | **245** |

## Sample errors: 0

All 13 sampled reviewer `match` rows are right in address and name. The reviewer was also right on the 5 Animation*
body counterparts and on RemoveObjectFromDialog. One non-sampled reviewer `match` is wrong as shipped: Destructor_3
(0x4727b0 -> 0x7ed5f0). The reviewer itself called it a split fragment and a duplicate, but still ruled `match`. It does
not count, because it was not sampled.

## The 14 address changes (final Steam address != pre-Opus `final_rows.tsv`)

All are part r rows. Each one is the unique fp-exact Steam body of its GOG function.

| GOG | name | pre-Opus pick | final | verdict |
|---|---|---|---|---|
| 0x42b680 | MenuActionSetObjectMine | none | 0x77f660 | label_suspect (= CSWCCreature::PerformCombatFeat) |
| 0x4726c0 | ClientServerSyncHandler_Position | 0x405970 | 0x7ed560 | verified |
| 0x4774e0 | BarkBubble | 0x7f23f0 (SecuritySpike) | 0x77b630 | verified (one body for CSWCObject + CSWCCreature) |
| 0x663800 | GetMaxHitPoints | 0x4597f0 | 0x53ef70 | likely |
| 0x663830 | GetLockOrientationToObject | none | 0x53ef90 | likely |
| 0x6638c0 | GetFirstName | 0x4598b0 (Gob) | 0x600090 | label_suspect (= CSWSItem::GetFirstName) |
| 0x6969a0 | AddSwitchWeaponsAction | none | 0x57de70 | label_suspect (= CSWSCreature::Rest) |
| 0x6a2c90 | LipSync | none | 0x58a1b0 | likely (ICF-shared "return 0" stub) |
| 0x6b5760 | AIActionPauseDialog | none | 0x5d9570 | verified |
| 0x6b5880 | AIActionResumeDialog | none | 0x5d9690 | verified |
| 0x6bbb10 | AIActionSwitchWeapons | none | 0x5df920 | verified |
| 0x6bd120 | AddUseObjectAction | 0x5564b0 | 0x5e0f30 | verified |
| 0x6bd190 | AddGiveItemActions | 0x5564b0 | 0x5e0fa0 | verified |
| 0x6bd340 | AddTakeItemActions | 0x5564b0 | 0x5e1150 | verified |

The 5 Animation* rows keep their gate pick (the B0 semantic address); only their verdict moves label_suspect -> verified.

## Lead notes, checked

1. **Animation* predicates.** These are verified. Steam 0x6eb670 stores 14 `animations.2da` column handles in
   alphabetical order, and each 129 B body reads one handle: Pause [0xa0fe2c] 0x7ef350, Walking [0xa0fe30] 0x7ef3e0,
   Running [0xa0fe34] 0x7ef470, Attack [0xa0fe4c] 0x7ef880, HideEquippedItems [0xa0fe50] 0x7ef910. PlayOutOfPlace
   0x7ef730 is the 68 B id-range test with no handle, which matches the Mac shape. GOG callers confirm against Mac: GOG
   0x4744b0 is called by AnimationUpdate/DriveAnimationUpdate (Mac Pause), TestAndReEquipweapons calls 0x474a70 (Mac
   HideEquippedItems), and AnimateFireAndForget calls 0x474890 (Mac PlayOutOfPlace).
2. **Shadows.** Verified: TurnOnShadows = 0x7ed4d0 (`007ed4ee MOV EAX,[EDX+0xb8]`), TurnOffShadows = 0x7ed500
   (`007ed51e MOV EAX,[EDX+0xbc]`); Mac +0xbc/+0xc0.
3. **Destructor split.** Verified: GOG 0x472750 (96 B) + 0x4727b0 (379 B) = Steam 0x7ed5f0 (475 B). Only Destructor_2
   holds 0x7ed5f0; Destructor_3 is rejected (part z). The deleting destructor (GOG 0x472720 -> 0x7ed5c0) is verified.
4. **CSWSObject member offsets.** Every one checked equals Mac exactly: GetPortraitId +0x30, IsDirtyFlagSet +0x224,
   GetMaxHitPoints +0xe4, AddMatchedExpressionString +0x1c4, the AIAction rows +0x11ac/+0x1124. No -4 shift in this batch.

## Label findings (GOG DB names; every Steam counterpart is fp 1:1)

1. **Animation* permutation** (GOG 0x4744b0/0x474540/0x4745d0/0x474890/0x474a70). The 5 labels are a permutation of the 5
   bodies, decided by column handle. B0 already ships the right Steam names (semantic overrides); B10a agrees.
2. **AddSwitchWeaponsAction chain.** The GOG AddSwitchWeaponsAction label (0x6969a0) sits on CSWSCreature::Rest. The
   AddDoorSaberAction label (0x6ae890) sits on AddSwitchWeaponsAction, and the real AddDoorSaberAction is `FUN_006ae820`
   -> Steam 0x547b40.
3. **Isolated errors.**
   - 0x42b680 MenuActionSetObjectMine = CSWCCreature::PerformCombatFeat. Mac has no MenuActionSetObjectMine. The GOG
     caller 0x427080 (`CSWCDoor::MenuActionSetDoorMine`, 113 B) does not fit Mac SetDoorMine (329 B) either; it is likely
     ActionMenuUseAttackFeat. That row is not in this batch.
   - 0x6638c0 GetFirstName = CSWSItem::GetFirstName (+0x2b0).
   - 0x6ad780 RemoveObjectFromDialog = IsAnimationPlayingInDialog forwarder; the real one is 0x6ad7e0 (`_2`).

All 5 label_suspect rows are in `gog_ledger_section.md` (`## B10a`).

## AUTO rotation audit (job 2)

15 AUTO rows were checked: the Animation* window plus +/-0x300 around each label_suspect row. 14 pass and 1 fails
(0x6ae890 AddDoorSaberAction -> label_suspect, part z). With the 8 part a meaning checks, 22 distinct AUTO rows of 161 had
an Opus meaning read; 1 was wrong.

## KDB (`kdb_overrides.tsv`, 6 rows)

- demote 0x405970 `ClientServerSyncHandler_Position`, 0x4597f0 `GetMaxHitPoints` (both wrong bodies, delta-map guesses),
  and 0x546aa0 `RemoveObjectFromDialog` (forwarder).
- rename 0x546b00 -> `CSWSObject::RemoveObjectFromDialog`, 0x547bb0 -> `CSWSObject::AddSwitchWeaponsAction`,
  0x5df920 -> `CSWSObject::AIActionSwitchWeapons` (it was the placeholder `CSWSCreature::AIActionType47_...`).

## B0 branch rows (57 of these classes): none wrong

All 57 were cross-checked against the B10a rows. All have the same Steam address as the B10a pick, and all are fp-equal
to their GOG body. The only differences are 4 rows with an empty-string ref count, plus the 2 semantic Animation rows
(Pause, PlayOutOfPlace), whose sizes differ on purpose. The 5 semantic Animation rows, TurnOn/OffShadows, both
GetPortraitId rows, SetId and SetFeedbackInfo were re-proved here.

## Expected final verdicts after `finalize B10a --opus-sample-errors 0`

Computed by applying overrides a + r + z (verdict and steam_addr) to `final_rows.tsv`:

| verdict | rows |
|---|---|
| verified | **210** |
| likely | 29 |
| label_suspect | 5 (0x42b680, 0x6638c0, 0x6969a0, 0x6ad780, 0x6ae890) |
| rejected | 1 (0x4727b0 Destructor_3) |
| unresolved | 0 |
| **total** | **245** |

- Moves from pre-Opus: likely -> verified 9; label_suspect -> verified 5; rejected -> verified 5; unresolved -> verified
  3; unresolved -> likely 2; unresolved -> label_suspect 2; rejected -> likely 1; rejected -> label_suspect 1; verified
  -> label_suspect 1 (0x6ae890); verified -> rejected 1 (0x4727b0).
- `functions.csv` (verified only) should go from 190 to **210**.
- **Duplicate check:** among the verified/likely/label_suspect rows, no Steam address is held by two names (pre-Opus
  0x7ed5f0 was held twice). None of the 14 new addresses is in the branch DB.
- **Coverage check:** all 14 REJECT/NOCAND rows and all 9 100%-rows have an override.

## Open items

1. Merge `opus/overrides_{a,r,z}.tsv` into `B10a/overrides.tsv` (46 rows), run `finalize B10a --opus-sample-errors 0`,
   and check the counts above.
2. `persist B10a --apply` (adds the `## Lessons` lines), apply `kdb_overrides.tsv`, `import-ghidra --mode append`,
   `SqliteTools validate`, `check-validated`. Paste `gog_ledger_section.md` into `../GOG_LABEL_SUSPECTS.md`.
3. Semantic extras (not shipped; the GOG rows carry other names): CSWSObject::AddSwitchWeaponsAction 0x547bb0,
   AddDoorSaberAction 0x547b40, CSWSCreature::Rest 0x57de70, CSWCCreature::PerformCombatFeat 0x77f660,
   CSWSItem::GetFirstName 0x600090. Decide by meaning (B0 `semantic:` precedent).
4. KPM / mod impact: a hook on BarkBubble 0x77b630 also fires for CSWCCreature::BarkBubble. A hook on LipSync 0x58a1b0
   fires for every function folded into that stub; do not hook it.
5. Below the batch: GOG 0x427080 `CSWCDoor::MenuActionSetDoorMine` (113 B, calls PerformCombatFeat) does not fit Mac
   SetDoorMine; check it in the CSWCDoor batch.
