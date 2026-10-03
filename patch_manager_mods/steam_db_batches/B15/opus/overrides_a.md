# B15 Opus gate, part A (opus_gate.md lines 14-121)

Scope: the 26 "Opus reads 100%" rows plus 77 sample rows. Lines 14-120 hold only 76 sample rows (lines 45-120), so line 121 (GetHasSpellEffect) is included to make 77; part B may overlap on that one row. OUT = `B15/opus/overrides_a.tsv` (103 rows).

## Method

- Re-decoded the command tables on my own from `server/cmdstores/*.cmdstores.tsv` (Windows slot = disp/4, Mac slot = disp/8; init functions are straight-line with no branches, so the later store of a slot wins). Steam: all 879 stores in the export equal the stores parsed from `00665f50.asm` + `006f5b80.asm`, line for line (including the slot-0 `006685f7 MOV dword ptr [ECX],0x68f5d0` store with no disp).
- Result: 877 slots on GOG, Steam and Mac; 655 GOG and 655 Steam handlers; every GOG slot set has exactly one identical Steam slot set; `cmdtable.tsv` agrees with my decode on 877 of 877 slots.
- For each row: fingerprint of the GOG fp against the Steam fp of the table counterpart (size, retn, n_insn, imm list, disp list, str_refs, and callees mapped through B0/B15 validated, pr3 verified and the table itself). **All 102 slot-backed rows are exact matches on every field.** The one row without a slot (0x78d6b0) also matches its Steam twin exactly.
- Named Mac symbols were checked per slot set, and Mac bodies were compared by shape (disp +4) for the name exceptions below.

## Table decode errors found: **0**

There is no slot where the decode is wrong. The one quirk is slot 156, which is stored twice on all three binaries (see below). The report already lists it.

## Points the lead asked about

- **Accepted decoy B15-4-38 (0x784680 -> 0x66f0a0): I agree the key is wrong.** 0x66f0a0 is the table handler for slots 173/412 (`00666496 MOV dword ptr [EDX + 0x2b4],0x66f0a0`, `006664c6 ... [EDX + 0x670]`). Its body tests the command id: `0066f0cd CMP dword ptr [EBP + 0x8],0xad` and `0066f0f5 CMP ...,0x19c`, where 0xad = 173 and 0x19c = 412. Its fp equals GOG (650 B). The "real" card 0x66f330 (113 B) is the ClearAllActions handler (slot 9). So the reviewer who accepted the decoy was right, and this row counts as neither a sample error nor a decoy failure.
- **GetEmotions (0x791A10): not a label error.** Slots 235-237 are nwscript GetIsEnemy/GetIsFriend/GetIsNeutral. The Mac symbol for that set is `ExecuteCommandGetEmotions`, and the Mac body has the same shape: it compares the command id with 0xeb/0xec/0xed, and disp 0x398 matches. It is a real source name that differs from the nwscript names. Final address 0x67c430 is verified.
- **IsAIState (0x7A0B00): not a label error.** Slot 140 is nwscript GetIsDead. The Mac symbol for slot 140 is `ExecuteCommandIsAIState`, and the Mac shape matches: disp 0x11ac, with vtable offsets 0x34/0x98/0xa0 on Mac against 0x30/0x94/0x9c on Windows. Final address 0x68b4b0 is verified (`0068b55f CMP dword ptr [EAX + 0x11ac],0x0`).
- **Gate candidates that differ from the table:** all 7 reviewer corrections equal the table counterpart, and so do my finals. Each wrong gate candidate is another slot's handler:
  - ChangeFaction: 0x66f330 is ClearAllActions (slot 9)
  - GetCreatureTalent: 0x67ac20 is GetCommandable (slot 163)
  - GetEmotions: 0x67c250 is GetEffectSpellId (slot 305)
  - GetMatchedStringsNum: 0x6837c0 is GetMetaMagicFeat (slot 105)
  - GetNearestObject: 0x6839a0 is GetModuleFileName/GetModuleName (slots 210/561)
  - IsAIState: 0x68b3c0 is IntToString (slot 92)
  - MoveToPoint: 0x68cb50 is ActionMoveToObject/ActionForceMoveToObject (slots 22/383)

## New finding: slot-156 pair, GOG names swapped against Mac

Slot 156 (nwscript EffectTemporaryForcePoints) is stored twice. The first store is dead and the later one runs:

| binary | first store (dead) | runtime store |
|---|---|---|
| GOG | `0077bd9d` -> 0x78d6b0 (282 B) | `0077c1cd` -> 0x78c440 (418 B) |
| Steam | `006667bd` -> 0x6780d0 | `00666bed` -> 0x676e60 |
| Mac | `0032566d` -> 0x333c92 `ExecuteCommandEffectTemporaryForcePoints` (165 B) | `00325c72` -> 0x337b80 `ExecuteCommandEffectTemporaryFP` (295 B) |

The bodies line up by shape. Mac runtime disp `0x8x5,0xa,0xa,0x34` matches GOG 0x78c440 `0x8x5,0xa,0xa,0x30`, and the Mac dead body has disp 0x8 only, like GOG 0x78d6b0. Against Mac, the GOG names of the two functions are therefore **swapped**.

- Both Steam addresses are correct counterparts (exact fp).
- 0x78c440 -> 0x676e60: **verified**. The GOG name fits the nwscript function this handler runs for.
- 0x78d6b0 -> 0x6780d0: **label_suspect**. The name EffectTemporaryFP is neither the nwscript name nor the Mac symbol of this body; the Mac symbol is ExecuteCommandEffectTemporaryForcePoints. The function is dead code, because the table overwrites its store.

## Overlap with overrides_b.tsv

Three GOG addresses are in both parts: 0x793c50, 0x78c440 and 0x78d6b0. The Steam addresses agree on all three. Only one verdict differs, on 0x78c440:

- Part B says label_suspect, because the Mac symbol is EffectTemporaryFP.
- Part A says verified, because the name fits the nwscript slot the handler runs for.

Either verdict is defensible. The lead should choose one convention for "source name differs from the nwscript name" and apply it the same way to GetEmotions and IsAIState.

## Side note (KDB, not my rows to fix)

`cg.py` names Steam 0x67f9b0 `Global::ExecuteCommandGetIsDoorActionPossible [medium]`. That is wrong: the table puts slot 405 GetIsDay there (`00667448 MOV dword ptr [ECX + 0x654],0x67f9b0`, and the body does `CMP ECX,0x1` on module+0x1dc). Re-check that KDB row when B15 is persisted.

## Per-row evidence

Columns: # (row), name, GOG address, gate/sample Steam address, final Steam address, verdict, slots. "fp=" means size, retn, n_insn, imm, disp, str_refs and mapped callees are all equal. The full anchor (store line plus body line) is in the note column of OUT.

| # | name | gog | row steam | final | verdict | slots |
|---|---|---|---|---|---|---|
| 0 | ChangeFaction | 0x00784680 | 0x0066f330 | 0x0066f0a0 (Opus corr) | verified | 173,412 |
| 1 | EffectAbilityDecrease | 0x00786c40 | 0x00671660 | 0x00671660 | verified | 446 |
| 2 | EffectAbilityIncrease | 0x00786e00 | 0x00671820 | 0x00671820 | verified | 80 |
| 3 | EffectMovementSpeedIncrease | 0x0078b150 | 0x00675b70 | 0x00675b70 | verified | 165 |
| 4 | EffectSpellLevelAbsorption | 0x0078bfe0 | 0x00676a00 | 0x00676a00 | verified | 472 |
| 5 | EffectForceResistanceDecrease | 0x0078c1c0 | 0x00676be0 | 0x00676be0 | verified | 454 |
| 6 | EffectForceResistanceIncrease | 0x0078c300 | 0x00676d20 | 0x00676d20 | verified | 212 |
| 7 | GetCreatureTalent | 0x00790420 | 0x0067ac20 | 0x0067ae40 (Opus corr) | verified | 307,308 |
| 8 | GetDistanceBetweenLocations | 0x00790ba0 | 0x0067b5c0 | 0x0067b5c0 | verified | 298 |
| 9 | GetEmotions | 0x00791a10 | 0x0067c250 | 0x0067c430 (Opus corr) | verified | 235,236,237 |
| 10 | GetFactionAverageLevel | 0x007920d0 | 0x0067caf0 | 0x0067caf0 | verified | 189 |
| 11 | GetGold | 0x00793430 | 0x0067de50 | 0x0067de50 | verified | 418,771 |
| 12 | GetIsDay | 0x00794f90 | 0x0067f9b0 | 0x0067f9b0 | verified | 405 |
| 13 | GetIsDusk | 0x007953a0 | 0x0067fdc0 | 0x0067fdc0 | verified | 408 |
| 14 | GetMatchedStringsNum | 0x00798d10 | 0x006837c0 | 0x00683740 (Opus corr) | verified | 195 |
| 15 | GetNearestObject | 0x00799310 | 0x006839a0 | 0x00683d40 (Opus corr) | verified | 38,226,227,228,229 |
| 16 | GetStringRight | 0x0079e650 | 0x00689000 | 0x00689000 | verified | 62 |
| 17 | IntToString | 0x007a0a10 | 0x0068b3c0 | 0x0068b3c0 | verified | 92 |
| 18 | IsAIState | 0x007a0b00 | 0x0068b3c0 | 0x0068b4b0 (Opus corr) | verified | 140 |
| 19 | MoveToPoint | 0x007a2530 | 0x0068cb50 | 0x0068cee0 (Opus corr) | verified | 21,382 |
| 20 | ShipBuild | 0x007a8290 | 0x00692c40 | 0x00692c40 | verified | 761 |
| 21 | Vector | 0x007aaac0 | 0x00695470 | 0x00695470 | verified | 142 |
| 22 | VectorMagnitude | 0x007aacd0 | 0x00695680 | 0x00695680 | verified | 104 |
| 23 | PauseGame | 0x007ac770 | 0x00697120 | 0x00697120 | verified | 57 |
| 24 | SetGalaxyMapPlanetAvailable | 0x007acb00 | 0x006974b0 | 0x006974b0 | verified | 742 |
| 25 | SaveNPCByObject | 0x007b1890 | 0x0069c2c0 | 0x0069c2c0 | verified | 873 |
| 26 | GetIdentified | 0x007940b0 | 0x0067ead0 | 0x0067ead0 | verified | 332 |
| 27 | GetStringLeft | 0x0079e2c0 | 0x00688c70 | 0x00688c70 | verified | 63 |
| 28 | DoSinglePlayerAutoSave | 0x00786680 | 0x006710a0 | 0x006710a0 | verified | 512 |
| 29 | DeleteJournalWorldEntryStrref | 0x00785c50 | 0x00670670 | 0x00670670 | verified | 674 |
| 30 | EffectAttackDecrease | 0x00787700 | 0x00672120 | 0x00672120 | verified | 447 |
| 31 | GetMaxForcePoints | 0x00794030 | 0x0067ea50 | 0x0067ea50 | verified | 56 |
| 32 | ResetDialogState | 0x007ae9d0 | 0x00699380 | 0x00699380 | verified | 749 |
| 33 | EffectACDecrease | 0x00786fc0 | 0x006719e0 | 0x006719e0 | verified | 450 |
| 34 | EffectBodyFuel | 0x0078d8d0 | 0x006782f0 | 0x006782f0 | verified | 224 |
| 35 | EffectDamageForcePoints | 0x0078d470 | 0x00677e90 | 0x00677e90 | verified | 372 |
| 36 | GetFactionStrongestMember | 0x00792b50 | 0x0067d570 | 0x0067d570 | verified | 182 |
| 37 | SetLockOrientationInDialog | 0x007ad6c0 | 0x00698070 | 0x00698070 | verified | 505 |
| 38 | GetFactionLeastDamagedMember | 0x00792630 | 0x0067d050 | 0x0067d050 | verified | 184 |
| 39 | EffectBlasterDeflectionDecrease | 0x0078d250 | 0x00677c70 | 0x00677c70 | verified | 470 |
| 40 | EffectVPRegenModifier | 0x0078a0a0 | 0x00674ac0 | 0x00674ac0 | verified | 780 |
| 41 | GetStrByStrRef | 0x0079e1b0 | 0x00688b60 | 0x00688b60 | verified | 239 |
| 42 | EffectDamageResistance | 0x00789600 | 0x00674020 | 0x00674020 | verified | 81 |
| 43 | GetDistanceToObject | 0x00791000 | 0x0067ba20 | 0x0067ba20 | verified | 41 |
| 44 | EffectHealForcePoints | 0x0078d590 | 0x00677fb0 | 0x00677fb0 | verified | 373 |
| 45 | EffectSavingThrowIncrease | 0x0078b7b0 | 0x006761d0 | 0x006761d0 | verified | 117 |
| 46 | GetFactionWeakestMember | 0x00792c60 | 0x0067d680 | 0x0067d680 | verified | 181 |
| 47 | EffectAttackIncrease | 0x007878e0 | 0x00672300 | 0x00672300 | verified | 118 |
| 48 | EffectFPRegenModifier | 0x00789f30 | 0x00674950 | 0x00674950 | verified | 779 |
| 49 | GetFactionAverageXP | 0x007922a0 | 0x0067ccc0 | 0x0067ccc0 | verified | 190 |
| 50 | EffectForceBody | 0x00789c80 | 0x006746a0 | 0x006746a0 | verified | 770 |
| 51 | PrintObject | 0x007a4850 | 0x0068f200 | 0x0068f200 | verified | 5 |
| 52 | SoundObjectStop | 0x007a8b70 | 0x00693520 | 0x00693520 | verified | 414 |
| 53 | EffectResurrection | 0x0078b420 | 0x00675e40 | 0x00675e40 | verified | 82 |
| 54 | EffectFactionModifier | 0x00789b10 | 0x00674530 | 0x00674530 | verified | 849 |
| 55 | EffectFury | 0x0078a210 | 0x00674c30 | 0x00674c30 | verified | 777 |
| 56 | SetCurrentStealthXP | 0x007ad080 | 0x00697a30 | 0x00697a30 | verified | 478 |
| 57 | SetLockHeadFollowInDialog | 0x007ad750 | 0x00698100 | 0x00698100 | verified | 506 |
| 58 | EffectBlasterDeflectionIncrease | 0x0078d130 | 0x00677b50 | 0x00677b50 | verified | 469 |
| 59 | GetIsXBox | 0x007b0f10 | 0x0069b940 | 0x0069b940 | verified | 851 |
| 60 | GetDistanceToObject2D | 0x007911a0 | 0x0067bbc0 | 0x0067bbc0 | verified | 335 |
| 61 | ExportAllCharacters | 0x0078e770 | 0x00679190 | 0x00679190 | verified | 557 |
| 62 | GetStringUpperCase | 0x0079e7c0 | 0x00689170 | 0x00689170 | verified | 60 |
| 63 | GetFactionBestAC | 0x00792380 | 0x0067cda0 | 0x0067cda0 | verified | 193 |
| 64 | GetStringLowerCase | 0x0079e520 | 0x00688ed0 | 0x00688ed0 | verified | 61 |
| 65 | GetChemical | 0x007935e0 | 0x0067e000 | 0x0067e000 | verified | 774 |
| 66 | GetFactionMostDamagedMember | 0x00792960 | 0x0067d380 | 0x0067d380 | verified | 183 |
| 67 | GetCheatCode | 0x007aee00 | 0x006997b0 | 0x006997b0 | verified | 764 |
| 68 | EffectTemporaryFP | 0x0078d6b0 | 0x006780d0 | 0x006780d0 | label_suspect | 156 (dead first store) |
| 69 | DeleteJournalWorldEntry | 0x00785b60 | 0x00670580 | 0x00670580 | verified | 673 |
| 70 | EffectMovementSpeedDecrease | 0x0078afc0 | 0x006759e0 | 0x006759e0 | verified | 451 |
| 71 | EffectACIncrease | 0x007871e0 | 0x00671c00 | 0x00671c00 | verified | 115 |
| 72 | EffectSkillIncrease | 0x0078bca0 | 0x006766c0 | 0x006766c0 | verified | 351 |
| 73 | EffectPsychicStatic | 0x0078d9c0 | 0x006783e0 | 0x006783e0 | verified | 676 |
| 74 | SoundObjectSetFixedVariance | 0x007a8950 | 0x00693300 | 0x00693300 | verified | 124 |
| 75 | GetGoldPieceValue | 0x00793530 | 0x0067df50 | 0x0067df50 | verified | 311,772 |
| 76 | PrintInteger | 0x007a4710 | 0x0068f0c0 | 0x0068f0c0 | verified | 4 |
| 77 | EffectSkillDecrease | 0x0078bae0 | 0x00676500 | 0x00676500 | verified | 453 |
| 78 | GetFactionAverageGoodEvilAlignment | 0x00791ff0 | 0x0067ca10 | 0x0067ca10 | verified | 187 |
| 79 | EffectTemporaryForcePoints | 0x0078c440 | 0x00676e60 | 0x00676e60 | verified | 156 |
| 80 | GetRunScriptVar | 0x0079d600 | 0x00687fb0 | 0x00687fb0 | verified | 565 |
| 81 | GetChemicalPieceValue | 0x007936e0 | 0x0067e100 | 0x0067e100 | verified | 775 |
| 82 | EffectSavingThrowDecrease | 0x0078b5a0 | 0x00675fc0 | 0x00675fc0 | verified | 452 |
| 83 | IsInTotalDefense | 0x007a1450 | 0x0068be00 | 0x0068be00 | verified | 812 |
| 84 | EffectDamageReduction | 0x007893f0 | 0x00673e10 | 0x00673e10 | verified | 119 |
| 85 | ActionEquipMostDamagingMelee | 0x0077f030 | 0x00669a50 | 0x00669a50 | verified | 399 |
| 86 | SoundObjectPlay | 0x007a88f0 | 0x006932a0 | 0x006932a0 | verified | 413 |
| 87 | SetMaxStealthXP | 0x007acf90 | 0x00697940 | 0x00697940 | verified | 468 |
| 88 | GetFactionGold | 0x00792550 | 0x0067cf70 | 0x0067cf70 | verified | 185 |
| 89 | IsStealthed | 0x007a1340 | 0x0068bcf0 | 0x0068bcf0 | verified | 810 |
| 90 | SoundObjectSetVolume | 0x007a8af0 | 0x006934a0 | 0x006934a0 | verified | 415 |
| 91 | EffectTemporaryHP | 0x0078c5f0 | 0x00677010 | 0x00677010 | verified | 314 |
| 92 | SoundObjectSetPitchVariation | 0x007a89d0 | 0x00693380 | 0x00693380 | verified | 690 |
| 93 | GetIsDawn | 0x00794f20 | 0x0067f940 | 0x0067f940 | verified | 407 |
| 94 | EffectHorrified | 0x0078d370 | 0x00677d90 | 0x00677d90 | verified | 471 |
| 95 | EffectInvisibility | 0x0078a9f0 | 0x00675410 | 0x00675410 | verified | 457 |
| 96 | GetLastAttacker | 0x00796e60 | 0x00681890 | 0x00681890 | verified | 36 |
| 97 | GetAttemptedSpellTarget | 0x0078f920 | 0x0067a340 | 0x0067a340 | verified | 375 |
| 98 | GetIsNight | 0x007956b0 | 0x006800d0 | 0x006800d0 | verified | 406 |
| 99 | EffectChoke | 0x0078d7d0 | 0x006781f0 | 0x006781f0 | verified | 159 |
| 100 | EffectForceSight | 0x00789df0 | 0x00674810 | 0x00674810 | verified | 823 |
| 101 | GetPlaceableIllumination | 0x0079cd60 | 0x00687710 | 0x00687710 | verified | 545 |
| 102 | GetHasSpellEffect | 0x00793c50 | 0x0067e670 | 0x0067e670 | verified | 304 |

## Counts

- Verdicts: verified 102, label_suspect 1, likely 0, rejected 0, unresolved 0.
- Opus corrections relative to the row steam_addr: 7. All are verified, and all equal the reviewer corrected_addr and the table counterpart.
- Reviewer rows in this part: all 97 `match` rows are on the final address, and every corrected_addr equals the final address.
- Table decode errors: 0.
- AUTO meaning checks: not in this part (opus_gate.md line 202 onward).

sample_errors: 0
