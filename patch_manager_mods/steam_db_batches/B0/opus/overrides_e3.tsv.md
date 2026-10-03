# B0 Opus gate, part e3 (102 escalated reviewer `match` rows)

Gate: Opus, 2026-10-03, following `OPUS_GATE_BRIEF.md`. Verdict rows are in `overrides_e3.tsv`.

Why these rows were escalated: B0 went to a 100% Opus read after the part-c sample found that the GOG label on 0x7c1880 contradicts its body. The main risk checked for each row was a wrong GOG label on a correct body counterpart.

## Method

1. **Fingerprints.** For every row I compared the GOG and Steam `fp.tsv` rows: size, retn, n_insn, disp, imm, callee count and string count.
2. **Callee identity.** I took each callee from the trusted mapping (`final_rows.tsv` or `steam_db_pr3/verified.tsv`), or found a Steam callee whose fingerprint equals the GOG callee's.
3. **Twins.** I looked for Steam fingerprint twins within 0x400 bytes.
4. **Label check against Mac** (scripted, then read by hand). I scored each GOG body against every Mac x86 function in the same class, using:
   - disp, with the CGameEffect field +0x30 counted as Mac +0x34;
   - imm, size and callee count.

   I flagged a row when the Mac function of the same name was not the best or near-best match. I also compared the GOG callee names with the Mac callee names of the same-name symbol.
5. **Anchors on disk.** Every asm line quoted in `overrides_e3.tsv` was checked in `steam_6a522e71/decomp`. 180 address-plus-mnemonic quotes were checked and all 180 are present as quoted. All reviewer-quoted lines for these rows were also checked, and none is missing.

## Results

- **Fingerprints:** 100 of 102 GOG/Steam pairs are identical. The other 2 differ only in `str_refs`:
  - **OnRemoveForcePush:** GOG str 0x99cdc4 is really the float constant 2.55 (0x40233333, the bytes `33#@`), which Ghidra read as a string. Mac has imm 0x40233333.
  - **GetDialogResref:** the Steam fp leaves out the empty-string ref, but `0053ef37 PUSH 0x986925` is there in the asm.
- **Callees:** every callee matches by trust or by fingerprint, except `operator new`, which is the CRT and has a different fp in each build.
- **Mac callee names:** these equal the GOG callee names on every row where Mac keeps the call. The only differences are CRT names and helpers that Mac inlined.
- **Mac shape:** the same-name Mac symbol is the best or near-best class match for 97 rows.
- **The 5 rows flagged by the Mac scoring** were settled by meaning:
  - **CSWSItem ctor:** Mac's ctor uses a different compiler layout. The Steam body stores `CSWSItem::vftable` and calls AddObject.
  - **CSWSItem::RemoveUpgrade:** the Mac disp 0x12, 0x28c and 0x294 is present, and the body compacts the list with memcpy.
  - **GetPortrait and SetPortrait:** the Mac bodies copy the same +0x20 resref, using 8-byte moves on Mac. SetPortrait is also reached through vt+0xcc from SetPortraitId (see below).
  - **GetAcceptableAction:** the Mac callers are the same two functions as on GOG and Steam.
- **Steam addresses:** none of the 102 is a wrong address, so there are no corrections.

Verdicts: **verified 95, likely 7, label_suspect 0, rejected 0, unresolved 0.**

The 7 `likely` rows:
- **Apply/RemoveBlasterBoltDeflectionDecrease:** the effect-type global 93 is shared by the pair. Whether 92 or 93 is the Increase type rests on GOG order only, as part C already decided.
- **Virtual accessors:** CSWSObject::GetDialogResref (an empty-resref stub), GetPortrait and GetPortraitId, and CSWSPlaceable::GetEffectSpellId and SetEffectSpellId.

## Rows needing more than fingerprint equality

- **Apply/Remove pairs.** Each pair is told apart by meaning, not position:
  - ADD vs SUB: ModifyNumAttacks and MaxDex.
  - Storing 1 vs 0: BodyFuel, LightsaberThrow and DoorSabering. For DoorSabering, Mac Sabering uses 0x2e0 and DoorCutting uses 0x2dc.
  - SetSilentMove(1) with `RET 0x14` vs SetSilentMove(0) with `RET 0x10`: DampenSound.
  - ApplyEffect vs RemoveEffectById, and the shared effect-type global: TrueSeeing, Disguise, Immunity, BonusFeat and BBD.
  - The list Add vs Remove callee: EffectList OnRemoveBonusFeat calls 0x5aa100 (fp= GOG 0x4c75e0). Mac OnRemoveBonusFeat calls `CExoArrayList<ushort>::Remove`, while Apply calls Add.
- **Improved vs Reduced saving throw.**
  - Reduced pushes cost table 0x14 (`005aea03`, `005aeac3`) and Improved pushes 0x1.
  - Mac ApplyReduced/RemoveReduced have imm 0x14; Mac ApplyImproved/RemoveImproved do not.
  - Apply vs Remove is told apart by the callee (ApplyChanged 0x5aeb20 vs RemoveChanged 0x5aecb0) and by `RET 0x14` vs `RET 0x10`.
- **Damage Reduction vs Resistance vs Vulnerability.**
  - Reduction and Resistance use the string 'Amount' (0x99626c; Mac 0x539001) with table 6 or 7. Mac imm 0x6 and 0x7 match.
  - Vulnerability uses 'Value' with table 0x16.
- **Decrease AC vs SkillModifier vs AbilityScore:** the tables are 0x14+5, 0x15 and 0x15, with the fields 0x13a, 0x164 and 0x150. These equal the Mac imm and disp.
- **ForceResistance Remove Increase vs Decrease:** the callee is SetForceResistance (0x6bc3d0) vs SetForceResistancePenalty (0x6bc400). These are trusted mappings and are the same callees as on Mac.
- **DispelAll vs DispelBest:** the stack frames are 0xc0 vs 0xb8, with list AddUnique vs Add and 19 vs 20 callees. Mac All is larger than Best, as on GOG.
- **ForceResisted vs ForceFizzle:** 0xfc5/0x27a1 vs 0xfc4. Mac has the same imm.
- **SetPortrait:** SetPortraitId 0x540b40 calls it through `00540ba0 MOV EDX,[EAX+0xcc]`. Mac SetPortraitId has disp 0xd0, which is the usual Mac vtable +4. This makes SetPortrait `verified`. GetPortrait and GetPortraitId stay `likely` (getters).
- **GetAcceptableAction:** the Steam callers 0x53f3c0 (209 B) and 0x67b100 (294 B) are the counterparts of the GOG callers GetActionByGroupId and ExecuteCommandGetCurrentAction. Mac `GetAcceptableAction` has the same two callers.
- **CSWSItemPropertyHandler destructors:**
  - 0x51d4e0 is the deleting dtor. It calls 0x5aa3c0, and its fp twin 0x51d4b0 is the EffectListHandler dtor.
  - 0x5aa3c0 stores the Handler vptr 0x992468 (`005aa3eb`) and restores the base vptr 0x99247c. This agrees with part C's finding on 0x7c1880 (the base dtor).

## CSWSPlaceable::SetEffectSpellId (GOG 0x4213a0 -> Steam 0x5ef180): shared ICF body, verdict `likely`

**The body is right.**
- It is `005ef18d MOV [EAX+0x3e0],ECX; RET 4`.
- It is the only such body in both builds. The only 25 B setter of 0x3e0 is GOG 0x4213a0 and Steam 0x5ef180.

**Only one caller exists, and it is the same in both builds.**
- GOG: `CSWCVisualEffectOnObject::ApplyLightsaberThrow` 0x4fec40 (1948 B).
- Steam: 0x873360 (1948 B).

**Mac uses this body as CSWCCreature::SetThrownLightsaber.**
- Mac ApplyLightsaberThrow calls `CSWCCreature::SetThrownLightsaber` 0x2870e4 (17 B, disp 0x3e0).
- Mac `CSWSPlaceable::SetEffectSpellId` (0x3d052a, disp 0x3e0) has no direct caller.

**Conclusion.** MSVC identical-COMDAT folding (ICF) merged the two setters.
- GOG kept the folded copy in the CSWCCreature block.
- Steam kept it next to the placeable getter, 0x5ef160.

Calling 0x5ef180 as SetEffectSpellId on a placeable is correct. Hooking it would see only client lightsaber-throw calls.

## GOG label errors found (for GOG_LABEL_SUSPECTS.md)

| GOG addr | GOG name | what it is | evidence |
|---|---|---|---|
| 0x4213a0 | `CSWSPlaceable::SetEffectSpellId` | also `CSWCCreature::SetThrownLightsaber` (ICF-shared body) | only caller is ApplyLightsaberThrow; Mac SetThrownLightsaber 0x2870e4 has disp 0x3e0 and is called by Mac ApplyLightsaberThrow |
| 0x4213c0 | `CSWCCreature::SetThrownLightsaber` | not SetThrownLightsaber: a SetArea helper that Mac inlined (name unknown) | 66 B, disp 0x20,0x20,0x108,0x3b4,0x3b4; GOG callers are CSWCCreature::SetArea 0x421370 and CSWCArea::LoadArea; Mac CSWCCreature::SetArea (59 B) has disp 0x20,0x108,0x3b4 |
| 0x42bd00 | `CSWCCreature::SetSWAnimationSpeed` | `CSWCCreature::GetLightSaberThrowRate` | 59 B, retn 0, disp 0x11dc, called by ApplyLightsaberThrow; Mac GetLightSaberThrowRate 0x28e084 (62 B, disp 0x11dc) is called by Mac ApplyLightsaberThrow; Mac SetSWAnimationSpeed uses 0x3c8/0x3cc |

Only the first of these is in this part, and it does not change a Steam address. The other two were seen while checking it; their Steam rows are not in e3 and were not gated.

Also seen, but owned by the pipeline and not this part:
- `steam_db_pr3/verified.tsv` maps both `GetPortraitId` (GOG 0x6638a0) and the `GetLastName` kludge (GOG 0x6a5ce0) to Steam 0x53f000.
- `B0/final_rows.tsv` already moves GetLastName to 0x53ef90.

## Per-row evidence (short)

"fp=" means the GOG and Steam fingerprints are identical. The full anchors are in the TSV notes.

| GOG | name | Steam | verdict | key anchor |
|---|---|---|---|---|
| 0x7420a0 | EffectList OnRemoveForceResistanceIncrease | 0x59f7d0 | verified | CALL 0x6bc3d0 SetForceResistance |
| 0x7423b0 | OnRemoveForceResistanceDecrease | 0x59fae0 | verified | CALL 0x6bc400 SetForceResistancePenalty |
| 0x744d20 | OnApplyLight | 0x5a2450 | verified | type 0x1e + ApplyEffect |
| 0x744e00 | OnApplyVision | 0x5a2530 | verified | temp 0x1e effect + ~CGameEffect |
| 0x744f10 | OnApplyModifyNumAttacks | 0x5a2640 | verified | ADD + SetBonusEffectAttacks |
| 0x745540 | OnApplySilence | 0x5a2c70 | verified | GetEffectImmunity, 0x91, +0xfd0=1 |
| 0x7457d0 | OnApplyDispelAllMagic | 0x5a2ee0 | verified | SUB ESP,0xc0; AddUnique |
| 0x745c80 | OnApplyDispelBestMagic | 0x5a3390 | verified | SUB ESP,0xb8; Add |
| 0x747810 | OnRemoveTimestop | 0x5a4f20 | verified | TogglePauseState + RemoveFromExclusionList |
| 0x748940 | OnApplyTurnResistance | 0x5a6050 | verified | [[+0x1198]+0x6c] (Mac) |
| 0x7489a0 | OnApplyDisappearAppear | 0x5a60b0 | verified | PacifyCreature, AddDisappearActions |
| 0x748b70 | OnRemoveDisappearAppear | 0x5a6280 | verified | ComputeSafeLocation, AddAppearActions |
| 0x748e60 | OnApplyAppear | 0x5a6570 | verified | ClearAllActions + AddAppearActions |
| 0x749540 | OnRemoveNegativeLevel | 0x5a6c60 | verified | Get/SetClassNegativeLevels |
| 0x749640 | OnRemoveBonusFeat | 0x5a6d60 | verified | ushort list Remove (Mac) |
| 0x749ec0 | OnApplyForceJump | 0x5a7610 | verified | 0x67, 0x3ea, SetLockOrientation |
| 0x74a0d0 | OnApplyForceJumpDelayed | 0x5a7820 | verified | AddMoveToPointActionToFront, +0xb08 |
| 0x74a720 | OnApplyForcePush | 0x5a7e70 | verified | CWalkHitInfo + TestDirectLine |
| 0x74acc0 | OnRemoveForcePush | 0x5a8410 | verified | 0x7f000000, 0xfffffea1, 2.55f (Mac) |
| 0x74ae10 | OnApplyDamageForcePoints | 0x5a8560 | verified | +0x130/+0x132, no max-FP call |
| 0x74af10 | OnApplyHealForcePoints | 0x5a8660 | verified | GetMaxForcePoints, 0xe4 |
| 0x74b0d0 | OnRemoveBodyFuel | 0x5a8820 | verified | +0x11c4 = 0 |
| 0x74b140 | OnApplyLightsaberThrow | 0x5a8860 | verified | +0x1128 = 1, 3187 B |
| 0x74bdc0 | OnRemoveLightsaberThrow | 0x5a94e0 | verified | +0x1128 = 0 |
| 0x74be00 | OnApplyForceResisted | 0x5a9520 | verified | 0xfc5, 0x27a1 |
| 0x74bf20 | OnApplyForceFizzle | 0x5a9640 | verified | 0xfc4 |
| 0x74c010 | OnApplyPureGoodPowers | 0x5a9730 | verified | SetCurrentHitPoints, +0x98/+0x9c |
| 0x74c8d0 | IsEffectToIgnoreOnDeath | 0x5a9ff0 | verified | 'EffectType' 2DA, 0x15/0x1e |
| 0x7e9150 | CSWSEncounter ctor | 0x622c90 | verified | AddObject, +0x320=0x7f000000 |
| 0x7e9520 | CSWSEncounter Destructor_2 | 0x623060 | verified | RemoveObject, ~CExoLocString |
| 0x7e9df0 | CSWSEncounter AddToArea | 0x623940 | verified | RemoveFromArea + AddObjectToArea |
| 0x7e9e80 | CSWSEncounter RemoveFromArea | 0x6239d0 | verified | RemoveObjectFromArea |
| 0x6634d0 | CSWSItem ctor | 0x5ffd60 | verified | CSWSItem::vftable 0x9973b0, AddObject |
| 0x6638e0 | CSWSItem Destructor | 0x6000b0 | verified | deleting dtor -> 0x6000e0 |
| 0x66d4e0 | CSWSItem AddGenderRequirements | 0x609cb0 | verified | PUSH 0x3c -> GetPropertyByTypeExists |
| 0x679380 | CSWSItem PowerItem | 0x615b50 | verified | GetClientItem + CSWCItem::PowerItem |
| 0x6796c0 | CSWSItem RemoveUpgrade | 0x615e90 | verified | +0x12 compare, memcpy compaction |
| 0x64cee0 | ItemPropertyHandler Destructor | 0x51d4e0 | verified | deleting dtor -> 0x5aa3c0 |
| 0x7c17d0 | ItemPropertyHandler Destructor_2 | 0x5aa3c0 | verified | vptr 0x992468 -> base 0x99247c |
| 0x7c1e20 | OnItemPropertyApplied | 0x5aaa10 | verified | table [this+4], 0x46 |
| 0x7c1e80 | OnItemPropertyRemoved | 0x5aaa70 | verified | table [this+8], 0x46 |
| 0x7c1ee0 | ApplyEnhancementBonus | 0x5aaad0 | verified | 'Value', GetDamageFlags, ApplyEffect |
| 0x7c2d90 | ApplyEnhancementPenalty | 0x5ab980 | verified | table 0x14 |
| 0x7c3800 | ApplyDamageBonus | 0x5ac3f0 | verified | [+0x9], 0xb/0xc |
| 0x7c3c80 | RemoveDamageBonus | 0x5ac870 | verified | +0x142 |
| 0x7c4070 | ApplyAttackBonus | 0x5acc60 | verified | 0x27/0x28 |
| 0x7c43d0 | RemoveAttackBonus | 0x5acfc0 | verified | 0x26-0x28, +0x13c |
| 0x7c47d0 | ApplyAttackPenalty | 0x5ad3c0 | verified | table 0x14 |
| 0x7c4ad0 | RemoveAttackPenalty | 0x5ad6c0 | verified | +0x13c, +0x122 |
| 0x7c4f00 | ApplyAbilityBonus | 0x5adaf0 | verified | 'Value', table 1, no NEG |
| 0x7c5100 | RemoveAbilityBonus | 0x5adcf0 | verified | +0x150 |
| 0x7c5a90 | RemoveImprovedForceResistance | 0x5ae680 | verified | +0x14e, table 0xb |
| 0x7c5c30 | ApplyImprovedSavingThrow | 0x5ae820 | verified | PUSH 1 + ApplyChanged |
| 0x7c5cf0 | RemoveImprovedSavingThrow | 0x5ae8e0 | verified | PUSH 1 + RemoveChanged |
| 0x7c5db0 | ApplyReducedSavingThrow | 0x5ae9a0 | verified | PUSH 0x14 + ApplyChanged (Mac 0x14) |
| 0x7c5e70 | RemoveReducedSavingThrow | 0x5aea60 | verified | PUSH 0x14 + RemoveChanged |
| 0x7c5f30 | ApplyChangedSavingThrow | 0x5aeb20 | verified | +0x122, callee of both wrappers |
| 0x7c6260 | ApplySpecialWalk | 0x5aee50 | verified | AND 0xffffffe7 |
| 0x7c65c0 | ApplyTrueSeeing | 0x5af1b0 | verified | type 0x99603c |
| 0x7c66a0 | RemoveTrueSeeing | 0x5af290 | verified | CMP type 0x99603c |
| 0x7c6770 | ApplyFreedomOfMovement | 0x5af360 | verified | 4 effects, 0x84 frame |
| 0x7c6b20 | RemoveFreedomOfMovement | 0x5af710 | verified | +0x16a |
| 0x7c6c70 | ApplyBonusFeat | 0x5af860 | verified | +0x1198, type 0x996068 |
| 0x7c6d90 | RemoveBonusFeat | 0x5af980 | verified | type 0x996068 |
| 0x7c6e90 | ApplyDamageImmunity | 0x5afa80 | verified | 'Value', damage-bit SHL |
| 0x7c7020 | RemoveDamageImmunity | 0x5afc10 | verified | 0xd compare |
| 0x7c72b0 | ApplyDamagePenalty | 0x5afea0 | verified | table 0x14, GetDamageFlags |
| 0x7c75f0 | RemoveDamagePenalty | 0x5b01e0 | verified | +0x13c, GetDamageFlags |
| 0x7c7900 | ApplyDamageReduction | 0x5b04f0 | verified | 'Amount', PUSH 6 |
| 0x7c7a70 | RemoveDamageReduction | 0x5b0660 | verified | +0x13e, PUSH 6 |
| 0x7c7c30 | ApplyDamageResistance | 0x5b0820 | verified | 'Amount', PUSH 7 |
| 0x7c7da0 | RemoveDamageResistance | 0x5b0990 | verified | +0x140, PUSH 7 |
| 0x7c7f60 | ApplyDamageVulnerability | 0x5b0b50 | verified | PUSH 0x16 |
| 0x7c80d0 | RemoveDamageVulnerability | 0x5b0cc0 | verified | PUSH 0x16 |
| 0x7c8270 | ApplyDecreaseAbilityScore | 0x5b0e60 | verified | NEG, table 0x15 |
| 0x7c8470 | RemoveDecreaseAbilityScore | 0x5b1060 | verified | +0x150, 0x15 |
| 0x7c8730 | ApplyDecreaseAC | 0x5b1320 | verified | 0x14 + PUSH 5, NEG |
| 0x7c88d0 | RemoveDecreaseAC | 0x5b14c0 | verified | +0x13a |
| 0x7c8ac0 | ApplyDecreaseSkillModifier | 0x5b16b0 | verified | 0x15, NEG |
| 0x7c8c50 | RemoveDecreaseSkillModifier | 0x5b1840 | verified | +0x164 |
| 0x7c8e40 | ApplyImmunity | 0x5b1a30 | verified | 10-case switch, type 0x995f74 |
| 0x7c9110 | RemoveImmunity | 0x5b1d00 | verified | CMP type 0x995f74 |
| 0x7c9340 | ApplySkillBonus | 0x5b1f30 | verified | +0x122 |
| 0x7c9490 | RemoveSkillBonus | 0x5b2080 | verified | +0x13c, +0x1198 |
| 0x7c95d0 | ApplyRegeneration | 0x5b21c0 | verified | 0x1770, type 7 |
| 0x7c9a60 | ApplyBlasterBoltDeflectionDecrease | 0x5b2650 | likely | type 93; Inc/Dec by order |
| 0x7c9b90 | RemoveBlasterBoltDeflectionDecrease | 0x5b2780 | likely | type 93; Inc/Dec by order |
| 0x7c9c80 | ApplyDisguise | 0x5b2870 | verified | type 0x996014 |
| 0x7c9d90 | RemoveDisguise | 0x5b2980 | verified | CMP type 0x996014 |
| 0x7c9e80 | ApplyDampenSound | 0x5b2a70 | verified | SetSilentMove(1), RET 0x14 |
| 0x7c9ef0 | ApplyDoorSabering | 0x5b2ae0 | verified | +0x2e0 = 1 (Mac) |
| 0x7c9f80 | ApplyMaxDexBonus | 0x5b2b70 | verified | ADD +0x2ec |
| 0x7c9fd0 | RemoveMaxDexBonus | 0x5b2bc0 | verified | SUB +0x2ec |
| 0x6637e0 | CSWSObject GetDialogResref | 0x53ef30 | likely | empty-resref virtual stub |
| 0x663840 | CSWSObject GetPortrait | 0x53efa0 | likely | getter of the +0x20 resref |
| 0x663870 | CSWSObject SetPortrait | 0x53efd0 | verified | called via vt+0xcc from SetPortraitId (Mac 0xd0) |
| 0x6638a0 | CSWSObject GetPortraitId | 0x53f000 | likely | getter +0x30 |
| 0x6b3380 | CSWSObject GetAcceptableAction | 0x5d7190 | verified | 0x46 switch; same 2 callers as Mac |
| 0x4213a0 | CSWSPlaceable SetEffectSpellId | 0x5ef180 | likely | ICF-shared with CSWCCreature::SetThrownLightsaber |
| 0x769550 | CSWSPlaceable GetEffectSpellId | 0x5ef160 | likely | getter +0x3e0 |
| 0x770e40 | CSWSPlaceable GetClientPlaceable | 0x5f6a70 | verified | GetClientObject + vt+0x4c |
| 0x7f00a0 | CSWSScriptVarTable GetFloat | 0x5e6810 | verified | MatchIndex type 2, FLD [+0xc], RET 4 |

## AUTO meaning checks
None: part e3 has no AUTO rows.

## Sample
Part e3 contains no rows from "Sample of the other reviewer matches".

sample_errors: 0
