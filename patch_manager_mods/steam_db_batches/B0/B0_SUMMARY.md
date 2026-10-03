# B0 summary: re-audit of the 1,704-row branch Steam DB (2026-10-03)

Inputs: `evidence.tsv` (post-G8 gates), `review/sonnet_c1..c14.tsv`, the Opus gate parts `opus/overrides_{a,b,c,e1,e2,e3,e4,f1,f2}.tsv`
(+ `.md` evidence notes), `calibration/OPUS_REVIEW.md`. Branch DB (read-only):
`~/Games/KOTOR2_Mod_Workspace/tools/kpm-pr-steam-db/AddressDatabases/kotor2_steam_aspyr.db` (1,704 rows, matched 1:1 by name;
8 rows matched through the renames listed below).

Outputs written: `overrides.tsv` (778 rows), `semantic_extra.tsv` (16 rows), finalize outputs (`final_rows.tsv`,
`functions.csv`, `kdb_upsert.json`, `branch_deletes.tsv`, `branch_corrections.csv`, `gog_label_suspects.tsv`,
`recheck.tsv`, `opus_gate.md/json`), and `../GOG_LABEL_SUSPECTS.md` (rewritten). The pre-Opus draft finalize outputs
were overwritten. Command: `python3 tools/batch.py finalize B0 --opus-sample-errors 1`. `persist` was run as a
dry run only.

## Final verdicts (finalize)

| verdict | rows |
|---|---|
| verified | 1571 |
| likely | 98 |
| label_suspect | 15 |
| rejected | 4 |
| unresolved | 16 |
| **total** | **1704** |

`functions.csv` = 1571 verified rows (likely rows ship only with `--with-likely`). The Opus gate is `ESCALATE` with
`clean: false`, because part c found 1 sample error (0x7c1880). The escalation was carried out: parts e1-e4 read 100%
of the reviewer matches, and f1/f2 read the AUTO rows near label errors plus the G8 flips. Decoys: 140 cards, 0 accepted.

## How the overrides were merged

- The 9 part files hold 774 rows, one per GOG address. No two parts wrote the same GOG address.
- **e2 vs f2 on GOG 0x6f5630** was a disagreement in the notes, not in the TSV rows. e2's "body = GetCreatureTalentRandomKnownPower"
  rested on disp values. f2's "body = UnReadySpell" holds, and it was checked on disk:
  - Steam 0x6b91b0 calls 0x6c1450 (GetSpell) and 0x54a610 (SetCurrentHitPoints) and makes no rand call;
  - its sole caller is `00581909 CALL 0x006b91b0` in 0x5818a0 (DecrementSpellReadyCount);
  - the real KnownPower 0x6bcfa0 calls rand (`006bd6fc CALL 0x0091eb8b`) and is called from `006bae97` inside the real
    TalentRandomPower 0x6bae00.
- Added by the merge (`[merge]` notes):
  - 3 AUTO rows that no part covered are confirmed label errors (e2 finding 4, f2 side finding). They are now
    `label_suspect`: 0x6f5220 DecrementFeatRemainingUses_2, 0x6f7280 SetArcaneSpellFailure and 0x6f7340
    GetTotalDamageRange. The pre-Opus draft had shipped all three as verified.
  - 0x720f20 `CSWSArea::Destructor_3` is `rejected`: it is not a function (the 3rd chunk of ClearLineOfSight, e2 finding 3).
- **37 semantic rows** (`semantic:` notes): for each name whose GOG DB address holds the wrong body, the row ships the
  Steam function proven to carry the name (Mac symbol + Steam anchor). Each one replaces that address's part row.
  - 34 are verified and 3 likely (RemoveGUIPopUps, IsAnimationPlayingInDialog and IsCameraDialog, all thin or
    shape-only).
  - 13 confirm the branch address: the branch was right by meaning, and only the GOG reference is wrong.
  - 24 change it.
  - Two semantic anchors were added on disk during the merge:
    - IsFadingInDialog 0x7cb720: `007cb736 CALL 0x008bb200` = CSWGuiDialogLetterbox::IsFadeDone (GOG 0x5af950 at
      SetFade's run delta), returns `!IsFadeDone`.
    - SpecialAbility 0x6baec0: `006b8ca4` call from GetCreatureTalentRandom, and `006baf9a [ECX+0x134]`.
- All 34 asm anchors cited by the semantic rows were re-checked in `steam_6a522e71/decomp`; all are present.
- 3 rows are tagged `ICF fold` in the note: 0x72fb00 GetClientOptions/GetGameOptions, 0x5ef180
  SetEffectSpellId/SetThrownLightsaber and 0x59aa40 OnRemove* stubs. Each ships under one name only.

## Uniqueness (R4)

Checked by script on `functions.csv` and on all verified + likely rows of `final_rows.tsv` (1669):
- no Steam address appears under two names;
- no name appears twice.

The branch DB itself had 7 shared addresses: 0x844e40, 0x53f000, 0x599d90, 0x5a1c10, 0x5a2e90, 0x5a4f20 and 0x5a8820.
B0 resolves all of them.

## Corrections vs the branch DB

| change | rows |
|---|---|
| same address, shipped (verified 1535 + likely 96) | 1631 |
| **address changed** (verified 36 + likely 2) | **38** (24 semantic, 14 body corrections) |
| **removed** (label_suspect 15 + rejected 4 + unresolved 16) | **35** |
| semantic rows (24 of them in "address changed") | 37 |
| name differs from the branch DB name (same address) | 8 |

### Address changes (branch id: name, branch -> B0)
- Body corrections:
  - 460 OnLoadGamePicked 0x8827a0->0x8824f0
  - 519 CSWSItem::GetUpgradableType 0x746fc0->0x6076d0
  - 633 GetClassByPosition 0x844e40->0x89aef0
  - 959 InGameInventory::OnControlSelected 0x8a83d0->0x8a8480
  - 1043 DEBUGDumpCombatRoundToScreen 0x593090->0x593000
  - 1101 LoadRoomInfo 0x5259b0->0x5258a0
  - 1102 LoadRooms 0x525bf0->0x5259b0
  - 1471 CSWSEncounter::AIUpdate 0x623320->0x623330
  - 1522 OnApplySummonParty 0x599d90->0x599a10
  - 1565 OnApplyForceResistanceIncrease 0x59f6f0->0x59f710
  - 1581 OnApplyInvisibility 0x5a1c10->0x5a1c50
  - 1592 OnApplyDispelAllMagic 0x5a2e90->0x5a2ee0
  - 1608 OnApplyTimestop 0x5a4f20->0x5a4d70
  - 1626 OnApplyNegativeLevel 0x5a65c0->0x5a65d0
- Semantic:
  - 464 IsAnimationPlayingInDialog 0x7cb6e0->0x7cadb0 (likely)
  - 465 RemoveGUIPopUps 0x7d0560->0x7d0620 (likely)
  - 644 GetForceResistance 0x6a7d40->0x6bad10
  - 648 GetWeaponFinesse 0x6a8ea0->0x6b8da0
  - 649 GetWeaponSpecialization 0x6a8f10->0x6b8ee0
  - 650 GetUseMonkAttackTables 0x6a8fe0->0x6a8ea0
  - 711 UnReadySpell 0x6b8ee0->0x6b91b0
  - 713 GetCreatureTalentRandomSpecialAbility 0x6b91b0->0x6baec0
  - 714 GetCreatureTalentRandomPower 0x6b9600->0x6bae00
  - 741 CheckInclusionMatch 0x6bc730->0x6bdd60
  - 750 GetIsInSpellLikeAbilityList 0x6be040->0x6ba8f0
  - 1061 SetHitInfoForCreature 0x54d3d0->0x556040
  - 1065 GridDFSTransTableInitialize 0x54f3d0->0x552a90
  - 1066 SmoothPointsOnPath 0x54f5e0->0x54f3d0
  - 1067 SmoothPointsOnPathRestartable 0x54fb10->0x54f5e0
  - 1068 SmoothSelectNodes 0x54fdb0->0x54fe40
  - 1069 SmoothSelectionRestartable 0x54fe40->0x54fb10
  - 1075 GridDFSTransTablePut 0x552a90->0x552c90
  - 1076 GridDFSTransTableGet 0x552b40->0x552b70
  - 1093 GetNextObjectInArea 0x523250->0x5232b0
  - 1094 GetFirstObjectInArea 0x5232b0->0x523250
  - 1095 SetMaxStealthXP 0x523320->0x52de00
  - 1194 ClearTilePathNodes 0x7a9950->0x7aa910
  - 1202 ClearAllMapPins 0x7aa910->0x7a96f0

### Semantic rows that keep the branch address
- 109 GetCanOrientInDialog 0x7cf710
- 123 IsCameraDialog 0x7cb750 (likely)
- 124 IsFadingInDialog 0x7cb720
- 225 AnimationHideEquippedItems 0x7ef910
- 229 AnimationPause 0x7ef350
- 230 AnimationPlayOutOfPlace 0x7ef730
- 231 AnimationRunning 0x7ef470
- 233 AnimationWalking 0x7ef3e0
- 463 GetMiniMapVisible 0x7d05a0
- 477 Gob::SetScene 0x461aa0
- 496 GetPropertyByTypeExists 0x603f60
- 497 RestoreUsedActiveProperties 0x603dd0
- 551 SetNumActiveProperties 0x604140

### Removed (exist in the branch DB, not shipped)
- **label_suspect (15):**
  - 21 CSWSCreatureStats::GetClass 0x844e40 (body = CSWCCreatureStats::GetClass)
  - 466 CGuiInGame::ShutDownGlobal
  - 710 DecrementFeatRemainingUses_2
  - 731 SetArcaneSpellFailure
  - 732 GetTotalDamageRange
  - 738 ComputeNumberKnownSpellsLeft
  - 749 AddSpellLikeAbilityToList
  - 854 GetSwoopUpgradeRowProvider
  - 1077 PlotSoundPath
  - 1078 PathPointDFSTransTableInitialize
  - 1155 GetClientArea_2
  - 1186 AddWeatherObject
  - 1187 GetWeatherObject
  - 1188 ClearWeatherObjects
  - 1655 CSWSItemPropertyHandler::Constructor
- **rejected (4):**
  - 270 CSWGuiBorderParams::SetFillImage 0x414840 (G6: address held by trusted CSWGuiControl)
  - 1137 CSWSArea::Constructor_2 (not a function)
  - 1138 CSWSArea::Destructor_3 (not a function)
  - 1360 CVirtualMachineStack::ClearStack 0x6fd6b0 (G4: Steam retn 0 vs GOG 12)
- **unresolved (16):**
  - 440 Scene::SetGeomFadeProperties
  - 469 CSWSObject::GetLastName
  - 473 CSWSPlaceable::GetDead
  - 476 Gob::SetProcessFlag
  - 504 CompareItem
  - 505 MergeItem
  - 506 SplitItem
  - 912 FreeResourceData_2
  - 935 CItemRepository::Constructor
  - 937 InsertItem
  - 938 AddItem
  - 939 FindItemWithBaseItemType
  - 940 FindItemWithTag
  - 1216 CSWSPlaceable::DoDamage
  - 1498 OnRemoveForceSight
  - 1645 OnApplyEffectImmunity

### Name differs from the branch DB (finalize uses GOG DB names)
| id | branch name | B0 name |
|---|---|---|
| 40 | CExoString::DefaultConstructor | Constructor |
| 41 | CExoString::CStrLenConstructor | Constructor_4 |
| 42 | CExoString::CStrConstructor | Constructor_2 |
| 44 | ConsoleFunc::IntConstructor | Constructor_3 |
| 45 | ConsoleFunc::StringConstructor | Constructor_2 |
| 46 | ConsoleFunc::NoParamConstructor | Constructor |
| 48 | Other::AurPostString | Global::AurPostString |
| 21 | CSWSCreatureStats::GetClass | CSWCLevelUpStats::GetClass (label_suspect) |

## Open items (lead decisions)

1. **8 rows were never read.** CSWSItem CompareItem, MergeItem and SplitItem, and CItemRepository ctor, InsertItem,
   AddItem, FindItemWithBaseItemType and FindItemWithTag (ids 504-506, 935, 937-940) were AUTO before the G8 re-gate.
   After it they became REVIEW (`why:c1`, run_ok yes -> n/a), but they were in no card chunk and no Opus part, so they
   are `unresolved` and would be removed. They need an Opus read before B0 ships.
2. **Removals that break KPM GameAPI on Steam.** `GameVersion::GetFunctionAddress` is called with these names:
   - CSWSCreatureStats::GetClass (21)
   - CGuiInGame::ShutDownGlobal (466)
   - CSWGuiBorderParams::SetFillImage (270, used by Snake)
   - CSWSObject::GetLastName (469)
   - CSWSPlaceable/CSWSObject::GetDead (473)
   - Gob::SetProcessFlag (476)
   - Scene::SetGeomFadeProperties (440)

   Decide: keep these rows flagged, or fix them first. In the branch, GetClass 0x844e40 is the client CSWCCreatureStats
   getter (+0xf8), so the GameAPI's server GetClass is probably wrong today.
3. **Renames break KPM API names.** The 7 rows in the table above are used by GameAPI CExoString/ConsoleFunc,
   EnableScriptAurPostString, K1SecuritySpikes and PartyRosterLimit32K1. The branch apply must keep the branch names
   for these rows; finalize's GOG names must not replace them.
4. **`persist` R4 skips (dry run: insert 23, update 1527, demote 1, skip 21).**
   - OnLoadGamePicked 0x8824f0 is blocked: the KDB has a trusted row at 0x8827a0. An Opus override naming 0x8827a0
     (OnLoadSaveGame) is needed to demote it.
   - Trusted-name conflicts keep the KDB names: 0x733570/0x733780 (CExoString), 0x7110d0 (CRes::CRes), 0x6076d0
     (GetUpgradeType vs GetUpgradableType) and 0x740fc0.
   - persist does not demote KDB names at label_suspect addresses. For example, 0x54d3d0 `SetHitInfoForCreature`
     [high] would remain next to the new 0x556040.
5. **`semantic_extra.tsv` (16 rows, not shipped):**
   - 13 proven names with no GOG DB row: AddMapPin, GetMapPin, GetPartyTable, CSWSCreatureStats::GetSpellsPerDayLeft,
     CSWCCreatureStats::GetClass, EvaluateOverlappingTargets, IncrementPlayersInArea, SmoothSelection, FindGobIdx,
     DialogGuiDoneScroll, ~CItemPropertyApplierRemover (likely), SetCurrentStealthXP (likely, shape only),
     CSWCCreatureStats::GetClassLevel (not gated).
   - 3 names whose GOG DB row exists but is not in B0: ShowDialogMessage 0x7d0560, OnLoadSaveGame 0x8827a0, and
     SetThrownLightsaber 0x5ef180 (ICF).
   - Decide whether to add them (new rows, so not B0 corrections).
6. **Ship likely?** 98 likely rows (thin getters/setters, the 3 thin semantic rows) are left out of
   `functions.csv` unless finalize runs with `--with-likely`.
7. **Still to locate (GOG name with no proven Steam body):** ResetInterfaceFont, DestroyDeathMenuGUI, ShutDownGlobal,
   DecrementFeatRemainingUses_2, SetArcaneSpellFailure, GetTotalDamageRange, AddSpellLikeAbilityToList,
   PlotSoundPath, PathPointDFSTransTableInitialize, the CSWSItemPropertyHandler ctor and GetLightSaberThrowRate. Also
   unchecked: ClassInfo::GetSpellGain 0x6e0580.
8. **Gate rule (part c):** demote an AUTO sig row to REVIEW when the same-name Mac body is outside 0.5x-2x of the GOG
   size and has a member disp the GOG body lacks, or when the GOG body never reads ECX. Not implemented. The 4 AUTO
   label errors above show the sig route cannot see GOG label errors.
9. Other unresolved rows (reviewer unsure or mismatch without a correction):
   - GetLastName: the branch's 0x53f000 is shared with GetPortraitId, and the reviewer rejects 0x53ef90.
   - OnApplyEffectImmunity: the branch's 0x5a8820 is shared with OnRemoveBodyFuel.
   - DoDamage, SetProcessFlag, GetDead, SetGeomFadeProperties, FreeResourceData_2 and OnRemoveForceSight.
