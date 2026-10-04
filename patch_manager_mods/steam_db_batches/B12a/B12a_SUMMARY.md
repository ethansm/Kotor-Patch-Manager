# B12a summary: CSWSCreature, GOG < 0x696F00 (2026-10-03)

Scope: 201 GOG rows, all class `CSWSCreature`, from GOG 0x653640 (AIActionPhysicalAttacks) to the B12a/B12b split at
GOG 0x696F00, plus one row (0x494D00 CanUseForcePowerOrSpell) that is really a client-stats method.

Inputs:
- `evidence.tsv`. Gates: AUTO 150 (route c2 110, sig 40), REVIEW 35, REJECT 14, NOCAND 2.
- `final_rows.tsv`. The pre-Opus finalize is at a12b504 (counts below). The working file is the 0ab0446 finalize with
  parts a+r.
- `review/sonnet_c1.tsv`: one Sonnet chunk, 49 cards (39 real, 10 decoys). Decoys accepted 0.
- `opus_gate.md/json`: **OK**. Opus reads the 100% classes plus a 20% sample (5 of 18) of the other reviewer matches.
- The Opus parts (63 GOG addresses; 0x696DB0 is in both a and r with the same Steam address and verdict, so there is no
  conflict):
  - `opus/overrides_a.tsv/.md`: 31 rows (verified 16, label_suspect 14, rejected 1), including 8 AUTO meaning checks
    (all pass). sample_errors 0.
  - `opus/overrides_r.tsv/.md`: 16 REJECT/NOCAND rows (verified 8, label_suspect 8). No sample rows.
  - `opus/overrides_z.tsv/.md`: 11 rows, made up of 3 part rows (2 mac_rotation 100% rows + 1 sample), 7 rows from the
    AUTO audit (5 in the rotation windows + 2 from a whole-batch sweep), and 1 note refresh of a part a row (0x68F7D0 =
    CancelAction). Verified 1, label_suspect 10.
    sample_errors 0.
- `overrides.tsv` holds a+r (46 rows). **Part z is not merged yet** (open item 1).
- `kdb_overrides.tsv`: 10 Opus renames of existing knowledge-DB rows (new for B12a).

This consolidation writes `opus/overrides_z.tsv/.md`, `kdb_overrides.tsv`, this file, a `## Lessons` section in
`gate_report.md` and a `## B12a` section in `../GOG_LABEL_SUSPECTS.md`. Nothing was run and no DB was written.

## Pre-Opus finalize (a12b504, for reference)

| verdict | rows |
|---|---|
| verified | 167 |
| likely | 6 |
| label_suspect | 10 |
| unresolved | 4 |
| rejected | 14 |
| **total** | **201** |

## Sample errors

**0** across all parts. Every sampled reviewer `match` had both the right address and the right name: GetLockOrientationToObject,
GetEffectSpellId, GetWeaponHitShots and SaveFollowInfo in part a, SetEffectSpellId in part z.

The reviewer still said `match` on wrong names in 100% rows, which are not counted as sample errors:
- 0x658710 GetWeaponDischargeTime and 0x65EF50 ResolveRangedMissToParry (mac_contra rows);
- 0x692510 SetAmbientAnimationState;
- 0x65DC30 ApplyOnHitSleep and 0x6614D0 ApplyOnHitHold. Here the reviewer wrote "name unverified" but still ruled
  `match`.

The reviewer's corrected addresses were right in every case.

## The 17 address corrections (final Steam address != gate pick)

Delta context: the gate's class-run delta (-0x118B30) is right for the 0x67xxxx-0x69xxxx part of the class. It is wrong
for the combat block GOG 0x6571B0-0x663170, whose true offset is Steam = GOG + 0x83D00 (+0x83D50 for
ResolveRangedAttack). Every 0x54xxxx pick in that block was wrong.

| GOG | name | gate pick | final | verdict | part |
|---|---|---|---|---|---|
| 0x494D00 | CanUseForcePowerOrSpell | none (NOCAND) | 0x84A170 | label_suspect (= CSWCCreatureStats::HasSpell) | r |
| 0x6571B0 | ResolveRangedAttack | 0x53E450 | 0x6DAF00 | verified | r |
| 0x65C680 | ResolveRangedMiss | 0x543AE0 | 0x6E0380 | label_suspect (anim-id predicate) | a |
| 0x65C710 | ResolveMeleeAnimations | 0x543AE0 | 0x6E0410 | verified | r |
| 0x65DA20 | CalculateMaxElementalDamage | 0x5452C0 | 0x6E1720 | verified | r |
| 0x65F2A0 | ResolveOnHitEffect | 0x546540 | 0x6E2FA0 | verified | r |
| 0x660930 | SavingThrowRoll | 0x547D60 | 0x6E4630 | verified | r |
| 0x661610 | ApplyOnHitConfusion | 0x548B10 | 0x6E5310 | verified | r |
| 0x6617B0 | ApplyOnHitFear | 0x548B10 | 0x6E54B0 | verified | r |
| 0x661950 | ApplyOnHitSlow | 0x548B10 | 0x6E5650 | label_suspect (= Hold) | r |
| 0x661B60 | ApplyOnHitPoison | 0x548DB0 | 0x6E5860 | label_suspect (= Sleep) | r |
| 0x662060 | ApplyOnHitSlayRacialGroup | none (NOCAND) | 0x6E5D60 | label_suspect (= Stun) | r |
| 0x6625D0 | ApplyOnHitInstantDeath | 0x6E6880 (GetFirstAttacker) | 0x6E62D0 | label_suspect (= Poison) | r |
| 0x662900 | ApplyOnHitKnockdown | 0x6E6AC0 | 0x6E6600 | label_suspect (= InstantDeath) | r |
| 0x663170 | SetAttackSummaryFeedbackEffectData | 0x6E71D0 (CTwoDimArrays ctor) | 0x6E6E70 | verified | r |
| 0x67C0C0 | Destructor | 0x563390 (= GetPortrait) | 0x563490 | verified | a |
| 0x68DDF0 | RemoveMindTrickEffects | 0x575340 (inner init) | 0x5752C0 | label_suspect (= CFollowInfo ctor) | a |

The pipeline gave 0x563390 to both GetPortrait and Destructor. Only GetPortrait keeps it.

## Label-rotation findings (GOG DB names; every Steam counterpart is fp 1:1)

1. **Combat block GOG 0x657710-0x6614D0 (Steam 0x6DB460-0x6E51D0): a multi-way rotation.** 12 labels sit on the wrong
   body. The full table is in `opus/overrides_z.md` ("Full combat-block table"), and the rows are in
   `../GOG_LABEL_SUSPECTS.md` `## B12a`. Ranged chain: the ResolvePostRangedDamage label is on ResolveRangedAnimations,
   the ApplyOnHitSleep label is on ResolvePostRangedDamage, the IsLightsaberEquipped label is on ResolveRangedMiss, the
   ResolveRangedMissToParry label is on IsLightsaberEquipped, and the ApplyOnHitSlayAlignmentGroup label is on
   ResolveRangedMissToParry. The ApplyOnHitHold label is on the 2-effect CreateLinkedEffect overload.
2. **ApplyOnHit* handlers (GOG 0x661950-0x662A30).** The ResolveOnHitEffect 12-case dispatch (Steam `006e3191 JMP
   [ECX*4+0x6e361c]`) gives each handler's identity. 5 labels are rotated, and 3 real handlers (Slow, SlayRacialGroup,
   Knockdown) are unlabelled FUN_ (part r table).
3. **Add*Actions shifted by one (GOG 0x680A70-0x680C50).** The PickUp body has the Drop label, the Drop body has the
   Heal label, and the Heal body has the `Rest` label (an AUTO row, caught by the audit). Mac order: PickUp, Drop, Heal.
4. **Isolated swaps:**
   - 0x693490 GetDetectMode = ResolveInitiative;
   - 0x68A4B0 ResolveInitiative = an `arg<10` predicate;
   - 0x693AC0 AddPickUpItemActions = ComputeArmourClass;
   - 0x696DB0 SetLastInstantEquip = RemoveMindTrickEffects;
   - 0x68DDF0 RemoveMindTrickEffects = CFollowInfo ctor;
   - 0x68F7D0 GetBlind = CancelAction;
   - 0x692510 SetAmbientAnimationState = an AIUpdate helper;
   - 0x657710 IsInTotalDefense_2 = GetClientRangedAnimation wrapper;
   - 0x692330 BroadcastForcePointUsageData = BroadcastStealthDetectionData (AUTO, sweep);
   - 0x696E80 Rest (batch name Rest_2) = GetRelativeWeaponSize (AUTO, sweep). Both GOG `Rest` labels are wrong; the
     `GetRelativeWeaponSize` label sits on 0x6F5300 = Stats::GetWeaponFocus (B12b).
5. **GOG 0x692F53 is a fragment** (0x692F50 + 3). It is rejected; the whole body is BroadcastPoisoned.

## AUTO rotation audit (part z, job 2)

37 AUTO rows were read by hand against the Mac symbol of the same name: the 31 inside the rotated windows (GOG
0x657000-0x663200 and 0x680A00-0x680D00) plus the 6 AUTO meaning checks of the regenerated `opus_gate.md` outside them.
32 pass. **5 fail** and get `label_suspect` overrides:
- 0x658540 ResolvePostRangedDamage (body = ResolveRangedAnimations);
- 0x6587E0 IsLightsaberEquipped (= ResolveRangedMiss);
- 0x65B530 ResolveRangedHit (= dice helper);
- 0x65E710 ApplyOnHitSlayAlignmentGroup (= ResolveRangedMissToParry);
- 0x680C50 Rest (= AddHealActions).

Then **all 150 AUTO rows** were swept by script: a same-name Mac score (disp + imm + callee-name Jaccard). The 18 low
scorers were read by hand, and **2 more fail outside the windows**:
- 0x692330 BroadcastForcePointUsageData (= BroadcastStealthDetectionData: `005798c1 PUSH 0x10`, sole caller
  DoSpotDetection);
- 0x696E80 Rest_2 (= GetRelativeWeaponSize: `0057e35c [EAX+0x53c]`, `0057e38c [ECX+0x1b]`, -10 out of range).

In total **7 of 150 AUTO rows had a wrong name** (5 in rotation windows, 2 isolated). Every one of the 150 had at least
the scripted meaning check, and at least 45 had a hand read (part a 8 + part z 37 - 2 overlaps + the 2 sweep finds).
Without the audit, all 7 would have shipped AUTO-verified under the wrong names. The KDB would also have kept, or
gained, the wrong names at 0x6DC290, 0x6DC530, 0x6DF230, 0x6E2410, 0x568020, 0x579800 and 0x57E350.

## Knowledge-DB rulings (`kdb_overrides.tsv`, 10 renames)

| Steam | KDB now | rename to |
|---|---|---|
| 0x6E51D0 | ApplyOnHitHold (medium) | CreateLinkedEffect_2 (Mac 2-effect overload; the 3-effect one is 0x6E5030) |
| 0x6DC290 | ResolvePostRangedDamage (medium) | ResolveRangedAnimations |
| 0x6DC530 | IsLightsaberEquipped (medium) | ResolveRangedMiss |
| 0x6E2410 | ApplyOnHitSlayAlignmentGroup (medium) | ResolveRangedMissToParry |
| 0x6E2C50 | ResolveRangedMissToParry (medium) | IsLightsaberEquipped |
| 0x6E1930 | ApplyOnHitSleep (medium) | ResolvePostRangedDamage |
| 0x57A960 | GetDetectMode (medium) | ResolveInitiative |
| 0x57C990 | GetNearestVisibleEnemyInRange (high) | GetNearestEnemy (Mac symbol) |
| 0x57E350 | Rest_2 (medium) | GetRelativeWeaponSize |
| 0x579800 | BroadcastForcePointUsageData (medium) | BroadcastStealthDetectionData |

- **0x57C990 needs the rename.** The trusted holder would otherwise block the batch's verified GetNearestEnemy upsert
  as a G6 CONFLICT.
- **0x563490 (stale GetPortrait, medium) is deliberately not listed.** The batch's verified Destructor row upserts
  0x563490 and replaces the untrusted name.
- The label_suspect step of `persist` demotes the remaining medium rows on its own, because each still carries the
  proven-wrong GOG name: 0x6DB460, 0x6DC460, 0x6DF230, 0x571980 and 0x5799E0.
- 0x5752C0 (CFollowInfo ctor) and 0x57E280 (RemoveMindTrickAndScrambleEffects) are high and already correct, so they
  are untouched.

## Expected final verdicts after `finalize B12a --opus-sample-errors 0`

These counts come from applying overrides a + r + z (the verdict and steam_addr of each) to `final_rows.tsv`:

| verdict | rows |
|---|---|
| verified | **171** |
| likely | 0 |
| label_suspect | 29 |
| rejected | 1 (0x692F53 fragment) |
| unresolved | 0 |
| **total** | **201** |

- Moves from pre-Opus:
  - verified -> label_suspect: 12
  - rejected -> verified: 9
  - likely -> verified: 6
  - rejected -> label_suspect: 5
  - unresolved -> label_suspect: 3
  - unresolved -> verified: 1
  - label_suspect -> rejected: 1
- Part z alone: verified -> label_suspect 9, likely -> verified 1 (SetEffectSpellId), and 1 note-only row.
- Tiers of the 171 shipped verified rows: opus 26, AUTO 133, REVIEW pipeline (unsampled reviewer match) 12.
- `functions.csv` (verified only) should go from 179 rows (current, a+r) to **171**: 9 rows drop out (0x6E1930,
  0x6E51D0, 0x6DC290, 0x6DC530, 0x6DF230, 0x6E2410, 0x568020, 0x579800, 0x57E350) and 1 is added (0x563370).
- **Duplicate check:** no Steam address is held by two verified rows.
- **Coverage check:** 39 of the 51 non-AUTO rows have an Opus override. The other 12 are unsampled REVIEW reviewer
  matches, which is the gate's design (decision OK, 20% sample). 2 of them (0x656DD0, 0x65A960) were also read in part
  z and pass.

## Open items

1. Merge: run `tools/merge_overrides.py B12a`, which takes `opus/overrides_z.tsv` (11 rows, tag `[z]`) into
   `B12a/overrides.tsv` (46 -> 56 rows; 0x68F7D0 replaces its part a note). Then run `finalize B12a
   --opus-sample-errors 0` and check the counts above. Expect decoys_accepted 0.
2. Run `persist B12a --apply`. It applies the 10 kdb renames and the `## Lessons` lines. Then run `import-ghidra --mode
   append`, `SqliteTools validate` and `check-validated`.
3. Semantic extras, not shipped (the GOG rows carry the wrong names), keyed by Steam address:
   - 0x6DC290 ResolveRangedAnimations
   - 0x6DC460 ResolveRangedHit
   - 0x6DC530 ResolveRangedMiss
   - 0x6DE7E0 GetWeaponDischargeTime
   - 0x6E1930 ResolvePostRangedDamage
   - 0x6E2410 ResolveRangedMissToParry
   - 0x6E2C50 IsLightsaberEquipped
   - 0x6E51D0 CreateLinkedEffect_2
   - 0x6E5650 / 0x6E5860 / 0x6E5D60 / 0x6E62D0 / 0x6E6600 / 0x6E5B10 / 0x6E6390 / 0x6E6730: Hold / Sleep / Stun /
     Poison / InstantDeath / Slow / SlayRacialGroup / Knockdown
   - 0x567E40 AddPickUpItemActions
   - 0x567F00 AddDropItemActions
   - 0x568020 AddHealActions
   - 0x57A960 ResolveInitiative
   - 0x57AF90 ComputeArmourClass
   - 0x576CA0 CancelAction
   - 0x579800 BroadcastStealthDetectionData
   - 0x57E350 GetRelativeWeaponSize

   Decide whether to ship them by meaning (B0 `semantic:` precedent).
4. To locate (shape only, not gated):
   - SetLastInstantEquip = GOG 0x6A3E00 -> 0x58B320 (B12b found it there too);
   - SetAmbientAnimationState = GOG 0x6A4260 -> 0x58B780;
   - the real CSWSCreature::Rest (Mac 792 B, 16 callees): both GOG `Rest` labels are on other bodies.
5. KPM / mod impact:
   - Any hook or call by the rotated combat names above runs a different function on GOG.
   - On Steam the KDB holds 9 of the combat names on the wrong bodies, plus GetDetectMode, ResolveInitiative,
     SetAmbientAnimationState, GetPortrait, Rest_2 and BroadcastForcePointUsageData (15 medium rows from
     gog_cluster_delta_map). Mods resolving `ApplyOnHitHold`,
     `ResolvePostRangedDamage` or `IsLightsaberEquipped` by KDB name would hit the wrong function until persist runs.
