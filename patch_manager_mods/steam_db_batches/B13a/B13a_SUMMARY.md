# B13a summary: CSWCCreature, the client-side creature (2026-10-03)

Scope: 199 GOG rows, class `CSWCCreature`, GOG 0x4102B0 (Constructor) to 0x42C070, plus one NOCAND row (0x443170
`SwitchMode`) that is not a CSWCCreature body.

Inputs:
- `evidence.tsv`. Gates: AUTO 119 (route c2 65, sig 54), REVIEW 79, REJECT 0, NOCAND 1.
- `final_rows.tsv`: pre-Opus finalize at 29bec26 (counts below).
- `review/sonnet_c1.tsv`, `review/sonnet_c2.tsv`: two Sonnet chunks, 20 decoys (10 per chunk), 0 accepted.
- `opus_gate.md/json`: OK at a 20% sample (11 of 52 reviewer matches) plus 26 rows read 100%.
  **Escalated**: Opus part a found 1 sample error, so every reviewer `match` got an Opus read (part z).
- Opus parts (disjoint, 91 GOG addresses, no conflict):
  - `opus/overrides_a.tsv/.md`: 45 rows (verified 30, label_suspect 13, unresolved 2), including the 8 AUTO meaning
    checks (all pass). sample_errors 1.
  - `opus/overrides_r.tsv/.md`: 1 NOCAND row (label_suspect, Steam 0x796B50 found by whole-binary fp scan).
  - `opus/overrides_z.tsv/.md`: 45 rows = 41 escalation rows + 4 AUTO audit rows (verified 40, label_suspect 5).
- `kdb_overrides.tsv`: 28 KDB name_map rulings (24 rename, 4 demote).

This consolidation writes the files above, this summary, a `## Lessons` section in `gate_report.md`, and
`gog_ledger_section.md` (the `## B13a` text for GOG_LABEL_SUSPECTS.md; that file was not edited). Nothing was
committed, finalized or persisted, and no DB was written.

## Pre-Opus finalize vs post-Opus (all three override files applied)

| verdict | pre-Opus | post-Opus |
|---|---|---|
| verified | 160 (AUTO 119 + REVIEW 41) | 177 (AUTO 115 + REVIEW 62) |
| likely | 15 | 0 |
| label_suspect | 12 | 19 (REVIEW 14, AUTO 4, NOCAND 1) |
| unresolved | 12 | 3 (0x41F6B0, 0x41F730 twins; 0x41F3D0 UpdateMonkVisuals, no Steam body) |
| rejected | 0 | 0 |
| **total** | **199** | **199** |

Not ruled by Opus: 107 AUTO rows (verified) and 1 REVIEW row (0x41F3D0, reviewer mismatch, stays unresolved).

## Sample errors

1, in part a: 0x41F7B0 `ResetDesiredPositionTimer` -> 0x7738E0. The address is right (fp 1:1). The body is the cdecl
`GetPath` callback (`0077f557 PUSH 0x98dac4` 'GetPath', `0077f55c PUSH 0x7738e0` in RegisterCallbacks). The real
ResetDesiredPositionTimer is GOG 0x414410 (labelled Unblur) -> Steam 0x7685F0 (`007685fa MOV [EAX+0x1f0],0x200`).

Not counted (not sampled), but also a reviewer `match` on a wrong name: 0x4137C0 and 0x422E00 (100% rows), and
0x413E30 (escalation row). The reviewer's address calls were right on every real card. Its name calls failed
wherever a GOG label sat on a neighbour's body.

## Address changes vs pre-Opus final_rows (9)

| GOG | name | pre-Opus | final | verdict | part |
|---|---|---|---|---|---|
| 0x410E50 | `scalar deleting dtor | 0x765110 (GetName body) | 0x765180 | verified | a |
| 0x420070 | GetHeadTurnHorizontal | 0x774200 | 0x7741A0 | verified | a |
| 0x4200D0 | GetHeadTurnVertical | 0x774260 (ApplyShadowBlob) | 0x774200 | verified | a |
| 0x421820 | GetCameraFollowPart | 0x7759B0 (GetPauseReadyAnimation) | 0x775930 | verified | a |
| 0x421840 | GetFollowingBody | 0x775970 | 0x775950 | verified | a |
| 0x421860 | GetHeadHeightOffset | 0x775990 | 0x775970 | verified | a |
| 0x421880 | GetCameraOffset | 0x775950 | 0x775990 | verified | a |
| 0x426490 | ActionMenuSwapImplantEnd | 0x77A4B0 (Agi) | 0x77A4D0 | verified | a |
| 0x443170 | SwitchMode (NOCAND) | none | 0x796B50 | label_suspect (= CClientExoAppInternal::IndicateObject) | r |

0x41F730 moves from 0x7737E0 to its positional 0x773860 and stays unresolved (twin rule). The pre-Opus
0x426430/0x426450 recheck corrections (0x77A470/0x77A490, PUSH 0x408/0x409) are confirmed.

## The label chain (why 19 label_suspect)

Every Steam counterpart in this class is positionally exact: the run delta drifts smoothly from 0x354340 to 0x353FE0,
and the alignment is 1:1 apart from GOG-only stubs. The errors are GOG labels sitting on neighbouring bodies, and they
form chains. A label's real body is usually another mislabelled row in the same batch:

- ResetAnimationState (Mac) = GOG 0x413E30, whose label ForceAppearanceUpdate belongs to 0x420720. 0x420720's label
  SetPopUpBubbleDecayTime belongs to a 5 B Mac stub.
- InCreatureCombat = 0x41BD70, labelled SetItemUnEquippedByMe. That body = 0x41ADE0, labelled GetLastInstant. That
  body = 0x41AC10, labelled UpdateRangedAttackData. That body = 0x41B7B0.
- IsDriveAnimationBlocked = 0x422E00, labelled InCreatureCombat. SetSWAnimationSpeed = 0x422DD0, labelled
  IsDriveAnimationBlocked. GetLightSaberThrowRate = 0x42BD00, labelled SetSWAnimationSpeed.
- StripEquippedItems and SetDesiredOrientationForLock are swapped (0x413620 vs 0x41F5A0). UnloadMonkVisuals =
  0x4137C0, labelled ClearVisibilityList, whose body = 0x41F840.
- Isolated: 0x413E70 (ComputeWeaponOverlays), 0x4213C0 (UpdateAreaId), 0x42BAE0 (stop rolling footstep sound), 0x4248C0
  (compare helper), 0x41AE50 (attack-index -> item helper), 0x41F7B0 (GetPath).

The opposite case also occurs. The thunk cluster 0x421820-0x421880 and the head-turn getters have correct GOG
labels, but the gate picks were rotated by one or two slots (mapper vote on identical 25 B thunks / similar getters).

## AUTO rotation audit

21 AUTO rows in the +/-0x300 windows were checked against Mac; 2 flagged (0x41ADE0 = SetItemUnEquippedByMe, 0x41F840 =
ClearVisibilityList). 2 more were found outside the windows while tracing the chains (0x41BD70 = InCreatureCombat,
0x420720 = ForceAppearanceUpdate). All 4 are `label_suspect` in part z. 0x412130 `AIUpdate` (47 B wrapper next to the
real body) was not checked.

## KDB (kdb_overrides.tsv, 28 rows)

The gog_cluster_delta_map rows in this class carried the rotated GOG labels, and some were off by a slot. 24 renames
to the Mac-shaped name, including the two the reviewer found: 0x768440 SetCombatState -> MoveCreatureToPosition and
0x77A9E0 AddStanceActions -> GetBehaviors. 0x796B50 SetHudTarget becomes CClientExoAppInternal::IndicateObject.
4 demotions where no Mac symbol fits (0x76EDA0, 0x76EFE0, 0x778900, 0x77FAC0). Each row quotes asm.

## Open items

1. Merge `opus/overrides_{a,r,z}.tsv` into `overrides.tsv`, then `finalize --opus-sample-errors 1`. This is the
   lead's job and was not run.
2. Twin rows 0x41F6B0/0x41F730: re-gate from the GOG RegisterCallbacks (0x42B260) PUSH order. The Steam side is pinned:
   0x7737E0 = GetPersonalRadius, 0x773860 = GetCreatureRadius.
3. New-name candidates that have no gated GOG row: 0x7685F0 ResetDesiredPositionTimer (GOG 0x414410), 0x776DB0
   CSWCCreature::SwitchMode (GOG 0x422D70, Mac disp 0x3C0 x3), 0x775A80 IsInjured (renamed in the KDB, not in the
   branch DB).
4. Copy `gog_ledger_section.md` into GOG_LABEL_SUSPECTS.md.
