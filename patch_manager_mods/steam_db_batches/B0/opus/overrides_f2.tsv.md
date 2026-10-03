# B0 Opus gate, part f2 (55 rows, no Sonnet review)

Gate: Opus, 2026-10-03, following `OPUS_GATE_BRIEF.md`. Verdict rows are in `overrides_f2.tsv`.

These rows come in two kinds:
- AUTO rows that sit within +-4 GOG positions of a confirmed GOG label error.
- Rows that the new G8 check flagged because a neighbour's body fits the Mac symbol better.

The main question for each row was whether the GOG name is right for the body.

## Method
For every row I did the following:
- Compared the GOG and Steam `fp.tsv` rows. On all 55 rows the size, retn, disp multiset and imm multiset are identical, checked by script. Every Steam address is therefore the body counterpart of its GOG address, except 0x720dc0, which is not a function (see below).
- Read the whole GOG class window (+-4 positions) next to the Mac x86 symbols of every name in it. I compared callee names, string sets and disp/imm shape (Mac vtable +4, Mac layout +4 in some classes) and built a fit matrix with rows for GOG bodies and columns for Mac symbols.
- For every name that looked wrong, ranked all Mac functions of the class against the GOG body, and all GOG functions of the class against the Mac symbol. I settled each such row with callers (GOG, Steam and Mac call edges) and the Steam asm.
- Checked every asm line quoted in the TSV against `steam_6a522e71/decomp` by script.

## G8 flags: 15 of the 16 are false positives; the exception is 0x720dc0
G8 rotation fits come from disp/imm overlap only. Callees and strings settle every flagged row:
- `LoadStores` has 'StoreList' and the CSWSStore callees.
- `SaveTriggers` has 'TriggerList'.
- The 3 `Load2DArrays_*` rows have their own table and column strings.
- `AddSwitchWeaponsAction` and `AddCutsceneMoveActions` write action type 0xe and 0xc, which matches Mac imm. AddSWItemSpellAction writes 0xa.
- `OnApplyAssuredHit` is RET 0xc and writes 1 to +0xfdc. Remove is RET 8.
- `SetLightIsOn` is RET 4 and writes +0x3e4/+0x3e8. Get takes no argument.
- `GetPropertyByType` has the full Mac disp set: 0x2, 0x11, 0x288-0x294, 0x2d4.
- `GetInActivateArea` has the same caller edge as on Mac, from UpdateActivateAreaList.
- `CopyFromStack` has ClearStack, memcpy and +0x3d4. The G8 alternative, SaveStack, has the GFF strings.
- `LoadVisibility` has '.vis' and sscanf "%s%d".
- `AddAction` is the +0xb00 list insert, called by the Add*Action functions.
- `AddToIntegerLocation` has no Mac symbol. The body is `if type[loc]==3: values[loc] += arg`, which fits the name.

The rotation_in flag on **0x720dc0 `CSWSArea::Constructor_2`** is real, but the cause is not a rotation:
- 0x720dc0 is the second Ghidra chunk of ClearLineOfSight 0x720d10. The chunk sizes are 176 + 352 + 239 = 767 B, which is the size of Steam 0x52bb60. Part e reached the same conclusion (ClearLineOfSight verified).
- The Steam address that the pipeline proposed for this row is the entry of ClearLineOfSight. No Steam function exists for this label.
- Verdict: `rejected`. The real CSWSArea ctor is GOG 0x716560 `CSWSArea::Constructor` (Mac CSWSArea::CSWSArea, fit 0.70).

## GOG label errors found (4 label_suspect + 1 non-function)

| GOG | GOG name | body is | Mac evidence | Steam of body | where the named function really is |
|---|---|---|---|---|---|
| 0x6f5360 | CSWSCreatureStats::UnReadySpell | **GetWeaponSpecialization**: `006b8f09 MOV CX,[EAX+0xb0]`, then HasFeat (`006b8f24 CALL 0x6b83f0`) | Mac GetWeaponSpecialization 0x3fbea4: 74 B, GetBaseItem+HasFeat, disp 0xb0 (GetWeaponFocus uses 0xb2). Callers Melee/Ranged/DamageBonus on both sides. Mac GetMeleeDamageBonus calls GetWeaponSpecialization at the spot where GOG calls 0x6f5360 | 0x6b8ee0 | UnReadySpell = GOG 0x6f5630 -> Steam 0x6b91b0 |
| 0x6f5630 | CSWSCreatureStats::GetCreatureTalentRandomSpecialAbility | **UnReadySpell** | Mac UnReadySpell 0x409c6a: callees 6/6 equal (GetSpell, GetAdjustedForcePoints, GetInteger, SetCurrentHitPoints, GetArea, GetRoomForceRating), disp 0xe8 x3. Its only caller is CSWSCreature::DecrementSpellReadyCount, the same edge as GOG 0x69a3d0 and Steam `00581909 CALL 0x6b91b0`. No rand call, so it is not a TalentRandom* picker | 0x6b91b0 | SpecialAbility = GOG 0x6f7340 -> Steam 0x6baec0 (part e2) |
| 0x6e41c0 | CSWSCreatureStats::GetForceResistance | `min(100, b[+0x128]+b[+0x129]+b[+0x12a])` (`006a7d6b CMP ECX,0x64`). Name unknown, and Mac has no function on those three bytes | Mac GetForceResistance 0x3fbac6 (27 B) = `[+0x12b]-[+0x12c]`. Windows SetForceResistance/Penalty write +0x12b/+0x12c (parts e2/e3 verified) | 0x6a7d40 | **GOG 0x6f7190 (unlabelled FUN_) -> Steam 0x6bad10**: `006bad20 MOVSX ECX,[EAX+0x12b]` / `006bad2a MOVSX EAX,[EDX+0x12c]` / `SUB`, clamped >= 0 |
| 0x6fa4c0 | CSWSCreatureStats::GetIsInSpellLikeAbilityList | Returns the class id of the last class slot: `006be081 MOV DL,[ECX+EAX+0xab]` with EAX=(n-1)*0x28, or 0xff when there are no classes. Name unknown. Mac inlines it (SetNormalBonusFlags and AddClass carry disp 0x8d, 0xab, imm 0x28) | Mac GetIsInSpellLikeAbilityList 0x408ae4 searches the SLA list at +0x134 with stride 0xc | 0x6be040 | **GOG 0x6f6d70 (unlabelled FUN_) -> Steam 0x6ba8f0**: SLA list +0x134, stride 0xc, `006ba958 CMP [EAX+ECX+0x4],1`, RET 4. GOG callers are AddCastSpellActions, ExecuteCommandCastSpell and GetCreatureHasTalent, the same as the Mac callers |
| 0x720dc0 | CSWSArea::Constructor_2 | not a function: 2nd chunk of ClearLineOfSight | - | inside 0x52bb60 (chunk start 0x52bc10) | the ctor is GOG 0x716560 |

**Correction to part e2, finding 4.** That finding identified the body at 0x6f5630 as GetCreatureTalentRandomKnownPower, based on disp values. The callees and the caller show that the body is **UnReadySpell**. The real GetCreatureTalentRandomKnownPower is the row's own label at GOG 0x6f9420 -> Steam 0x6bcfa0, which I verify in this part:
- It calls rand, CStr and the list Add, as Mac KnownPower does.
- It is called by the real TalentRandomPower GOG 0x6f7280 (Steam `006bae97`), as on Mac.

The row verdict (label_suspect) is the same either way. Only the "body is" name changes.

## Side finding (outside this part's rows, for the lead)
- **The real `GetWeaponFinesse` is GOG 0x6f5220 (labelled `DecrementFeatRemainingUses_2`) -> Steam 0x6b8da0.** The GOG and Steam fingerprints are equal: 217 B, RET 4.
  - Its callees GetBaseItem, IsLightsaber, GetDEXMod and HasFeat are 4/4 the Mac GetWeaponFinesse 0x3fbd3e callees.
  - Its disp (0x8 x3, 0xee) and its imm 0xc1/0xc2 match Mac exactly.
  - GetMeleeAttackBonus calls it at `006a7ed5`. Mac GetMeleeAttackBonus calls GetWeaponFinesse.
  - Part b had "Real GetWeaponFinesse: to locate"; this row locates it.
- Rows that can ship under the right names if the lead wants them:
  - `GetWeaponSpecialization` = 0x6b8ee0
  - `UnReadySpell` = 0x6b91b0
  - `GetForceResistance` = 0x6bad10
  - `GetIsInSpellLikeAbilityList` = 0x6ba8f0
  - `GetWeaponFinesse` = 0x6b8da0

  Each one is backed by a quoted asm line plus a Mac callee or caller identity.
- The CSWSCreatureStats weapon/talent block is a **multi-way rotation**, not isolated swaps. Combined with parts b and e2:
  - GetWeaponFinesse sits at the DecrementFeatRemainingUses_2 slot.
  - GetUseMonkAttackTables sits at the GetWeaponFinesse slot.
  - GetWeaponSpecialization sits at the UnReadySpell slot.
  - UnReadySpell sits at the TalentRandomSpecialAbility slot.
  - SpecialAbility sits at the GetTotalDamageRange slot.
  - TalentRandomPower sits at the SetArcaneSpellFailure slot.
  - GetSpellsPerDayLeft sits at the TalentRandomPower slot.

## Per-row evidence (short)
All rows are fp-equal between GOG and Steam. Each "Mac" note means the body fits the Mac symbol of the same name.

| # | GOG | name | verdict | anchor |
|---|---|---|---|---|
| 1 | 0x71ce60 | CSWSArea::LoadStores | verified | 'StoreList', CSWSStore callees |
| 2 | 0x71fb00 | CSWSArea::SaveTriggers | verified | 'TriggerList' |
| 3 | 0x720dc0 | CSWSArea::Constructor_2 | rejected | chunk of ClearLineOfSight |
| 4 | 0x722d30 | CSWSArea::ClearObjects | verified | +0x194/+0x198, 2/2 callees |
| 5 | 0x722de0 | CSWSArea::AwardStealthXP | verified | +0x2ec/+0x2f4, DistributeExperience |
| 6 | 0x722f60 | CSWSArea::EnforceNoPCHeadCloning | verified | 'NORMALHEAD' |
| 7 | 0x723140 | CSWSArea::SetFogColor | verified | write +0x8c (SunFogColor field) |
| 8 | 0x774920 | CSWSCombatRound::AddAction | verified | +0xb00 list insert, Add*Action callers |
| 9 | 0x775e90 | CSWSCombatRound::AddSwitchWeaponsAction | verified | type 0xe |
| 10 | 0x776060 | CSWSCombatRound::AddCutsceneMoveActions | verified | type 0xc |
| 11 | 0x6e41c0 | CSWSCreatureStats::GetForceResistance | label_suspect | see table |
| 12 | 0x6e4230 | GetMeleeAttackBonus | verified | GetBaseAttackBonus + feat imm |
| 13 | 0x6e47f0 | GetMeleeDamageBonus | verified | calls the real GetWeaponSpecialization, +0x53c |
| 14 | 0x6e50a0 | GetRangedDamageBonus | verified | same, imm 0xf0..0xf4 |
| 15 | 0x6e5580 | GetUnarmedDamageRoll | verified | 'DamageMult' |
| 16 | 0x6e5820 | GetAttackModifierVersus | verified | ' + %d (Base Attack Bonus)' |
| 17 | 0x6e83f0 | GetDamageBonus | verified | ' + %d (Offhand Strength Bonus)...' |
| 18 | 0x6e9340 | ResolveSpecialAttackAttackBonus | verified | identical disp set, 2/2 callees |
| 19 | 0x6f5360 | UnReadySpell | label_suspect | = GetWeaponSpecialization |
| 20 | 0x6f5630 | GetCreatureTalentRandomSpecialAbility | label_suspect | = UnReadySpell |
| 21 | 0x6f5ac0 | ValidateLevelUp | verified | CanLevelUp + LevelUp + event 0x25 |
| 22 | 0x6f5c10 | CanLevelUp | verified | GetLevel + GetServerInfo |
| 23 | 0x6f5cf0 | LevelUp | verified | AddFeat/AddKnownSpell/GetMaxForcePoints |
| 24 | 0x6f6200 | LevelDown | verified | UnequipItem/CanEquipItem |
| 25 | 0x6f8560 | GetCreatureTalentRandomFeat | verified | GetCreatureHasTalent, -100 |
| 26 | 0x6f88b0 | GetFeat_2 | verified | word array +0/+4, 0xffff |
| 27 | 0x6f89b0 | GetBonusFeat | verified | +0x18/+0x1c |
| 28 | 0x6f89f0 | GetEffectImmunity | verified | +0x122/+0x18a |
| 29 | 0x6f8e20 | GetEffectLinkImmunity | verified | SendFeedbackMessage, recursion |
| 30 | 0x6f9280 | AdjustSpellUsesPerDay | verified | GetSpellGainWithBonus + IsJedi |
| 31 | 0x6f9420 | GetCreatureTalentRandomKnownPower | verified | caller real Power, rand |
| 32 | 0x6f9c90 | GetSpellLikeAbilityCasterLevel | verified | SLA entry +0x8 |
| 33 | 0x6f9d10 | AdjustHPAccumulatorBy | verified | float +0x1bc |
| 34 | 0x6f9e40 | HasSpell | verified | caller CanChooseSpell |
| 35 | 0x6fa150 | AddClass | verified | +0x8d++, SetClassLevel |
| 36 | 0x6fa4c0 | GetIsInSpellLikeAbilityList | label_suspect | = last-class getter |
| 37 | 0x6fa520 | SetNormalBonusFlags | verified | IsBonusFeat/IsNormalFeat, +0x124 |
| 38 | 0x6fa5d0 | CanChooseFeat | verified | 'baodur', 'kreia' |
| 39 | 0x6faa70 | CanChooseSpell | verified | 4/4 callees |
| 40 | 0x73b770 | OnApplyAssuredHit | verified | +0xfdc=1, RET 0xc |
| 41 | 0x7ea040 | CSWSEncounter::GetInActivateArea | verified | caller UpdateActivateAreaList |
| 42 | 0x666f70 | CSWSItem::SaveContainerItems | verified | 'ItemList' |
| 43 | 0x667090 | UpdateUsedActiveProperties | verified | SendServerToPlayerUpdateActiveItemProperties |
| 44 | 0x667380 | GetUsedActivePropertyUsesLeft | verified | +0x298/+0x2cc |
| 45 | 0x667a90 | GetActiveProperty | verified | +0x288/+0x290 |
| 46 | 0x667ad0 | GetPassiveProperty | verified | +0x28c/+0x294 |
| 47 | 0x667b10 | GetPropertyByType | verified | +0x11/+0x2d4 |
| 48 | 0x667d40 | ApplyItemProperties | verified | OnItemPropertyApplied |
| 49 | 0x770810 | CSWSPlaceable::SetLightIsOn | verified | +0x3e4/+0x3e8, 'LightColor' |
| 50 | 0x481d80 | Load2DArrays_PersistentVisualEffect | verified | 'VFX_Persistent' |
| 51 | 0x482710 | Load2DArrays_GenericDoors | verified | 'GenericDoors' |
| 52 | 0x4830e0 | Load2DArrays_AmbientMusic | verified | 'AmbientMusic' |
| 53 | 0x641ad0 | CVirtualMachineStack::AddToIntegerLocation | verified | type==3 then add |
| 54 | 0x641b20 | CVirtualMachineStack::CopyFromStack | verified | +0x3d4, ClearStack, memcpy |
| 55 | 0x861e10 | Scene::LoadVisibility | verified | '.vis', sscanf |

| verdict | count |
|---|---|
| verified | 50 |
| likely | 0 |
| label_suspect | 4 |
| rejected | 1 |

## AUTO meaning checks (rows 4-7, 11-39, 42-46, 48: 39 AUTO rows)

| result | count | rows |
|---|---|---|
| pass | 35 | all others |
| fail | 4 | 0x6e41c0 GetForceResistance, 0x6f5360 UnReadySpell, 0x6f5630 GetCreatureTalentRandomSpecialAbility, 0x6fa4c0 GetIsInSpellLikeAbilityList |

All 4 failures are in the CSWSCreatureStats block. The Steam counterpart is right in every case; only the GOG name is wrong.

sample_errors: 0 (this part has no rows from the "Sample of the other reviewer matches" section)
