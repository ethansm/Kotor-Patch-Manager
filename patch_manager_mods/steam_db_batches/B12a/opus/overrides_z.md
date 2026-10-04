# Opus gate B12a part z: evidence

Method: GOG/Steam/Mac x86 `fp.tsv` rows (size, retn, n_insn, disp, imm, callees), GOG callee names, Steam asm quoted
from `steam_6a522e71/decomp`. "fp 1:1" = size, retn, n_insn, disp and imm all identical between GOG and Steam. All 11
rows below are fp 1:1, so every Steam counterpart is right. The question per row is the name, decided against the Mac
symbol by callee set and disp/imm (GOG callee labels inside the combat block are rotated too, so callee names were
mapped through the bodies, not the labels).

## Part rows

1. **0x65dc30 ApplyOnHitSleep -> 0x6e1930: label_suspect (body = ResolvePostRangedDamage).** fp 1:1 (808 B, RET 4, 11
   callees). The GOG callees are GetIsWeaponEffective 0x699780, GetTotalDamage 0x771940, CSWCCMessageData, GetAttack,
   CGameEffect/SetInteger/SetCreator and Add. In Steam: `006e1bb3 CALL 0x00580c50` (205 B, fp= GOG GetIsWeaponEffective),
   `006e1999 CALL 0x0058d290` (84 B, fp= GetTotalDamage), `006e1c10 PUSH 0x75`, `006e1c3d MOV [EDX+0xacc],1`, and the
   sole caller is ResolveRangedAttack `006db28c` (0x6daf00, part r). Mac `ResolvePostRangedDamage` (0x3713e6) has the
   same callee set, imm 0x34/0x75 and disp 0xabc/0xacc/0x10dc (score 0.83, the best Mac match). Mac `ApplyOnHitSleep`
   calls CreateLinkedEffect with imm 0x5e/0xcf, and this body has neither. Real Sleep = GOG 0x661b60 -> 0x6e5860
   (part r, dispatch case 0). The GOG label `ResolvePostRangedDamage` is on 0x658540 (ResolveRangedAnimations body).
   Reviewer `match` ("name unverified"): right address, wrong name.
2. **0x6614d0 ApplyOnHitHold -> 0x6e51d0: label_suspect (body = CreateLinkedEffect, 2-effect overload).** fp 1:1
   (305 B, RET 0xc, 5 callees). GOG callees CGameEffect ctor, SetInteger, SetLinked 0x734f20, SetCreator and new are
   exactly the Mac `CSWSCreature::CreateLinkedEffect` callee set. Steam: two `new 0x8c` + `CALL 0x005e4920`
   (CGameEffect(1)), `006e52af CALL 0x005e4e80` SetInteger(0,arg), `006e52e8 CALL 0x005e5260` SetLinked, `006e52fe RET
   0xc`. The 7 Steam callers (0x6e5860, 0x6e5d60, 0x6e5650, 0x6e5310, 0x6e54b0, 0x6e5b10, 0x6e5f00) are the bodies of
   Sleep, Stun, Hold, Confusion, Fear, Slow and AbilityDamage: the 7 Mac callers of CreateLinkedEffect. Mac has two
   overloads: 0x37807e (3x new 0x8c) = GOG 0x661330 -> 0x6e5030 (AUTO, label CreateLinkedEffect, correct) and 0x37817c
   (2x new 0x8c) = this row. Real Hold = GOG 0x661950 -> 0x6e5650 (part r). Not counted as a sample error (100% row).
3. **0x67bfa0 SetEffectSpellId -> 0x563370: verified (sample).** fp 1:1 and unique in both binaries (25 B, RET 4):
   `0056337d MOV dword ptr [EAX + 0xff8],ECX`, `00563386 RET 0x4`. Mac `CSWSCreature::SetEffectSpellId` 17 B, disp
   0xff8; the getter 0x563350 (GOG 0x67bf80, part a) reads +0xff8. No callers (virtual slot), as on Mac. Reviewer
   `match`: correct.

## AUTO rotation audit (job 2)

Scope: every AUTO row of `B12a/evidence.tsv` with GOG in 0x657000-0x663200 or 0x680a00-0x680d00, plus any AUTO row at
an isolated-swap GOG address (0x693490, 0x693ac0, 0x696db0, 0x68f7d0; none of them is AUTO): **31 rows**. Also the 6
AUTO meaning-check rows of the regenerated `opus_gate.md` that lie outside those windows (0x656d70, 0x656fa0,
0x67be50, 0x67bf20, 0x681f50, 0x694970) and the 2 unsampled REVIEW `match` rows in the window (0x656dd0, 0x65a960).
Each: GOG/Steam fp 1:1, GOG body compared with the Mac symbol of the same name (callee set, disp/imm). **37 AUTO rows
read by hand, 5 flagged**, all inside the combat-block and Add*Actions rotations. A scripted sweep of all 150 AUTO rows
then flagged 2 more outside the windows (see "Whole-batch sweep" below), for 7 in total. They are added to `overrides_z.tsv` as
`label_suspect` (overrides win over AUTO).

| GOG -> Steam | GOG name | check vs Mac symbol of that name | result |
|---|---|---|---|
| 0x657750 -> 0x6db4a0 | ResolveRangedSpecialAttack | 19 callees = Mac (SavingThrowRoll, GetFeatRemainingUses, ApplyEffect..), imm 0xe4-0xe6 | pass |
| 0x6583f0 -> 0x6dc140 | ResolveSafeProjectile | BroadcastSafeProjectile, GetItemInSlot, GetBaseDamage = Mac; 0x8e/0x1150 | pass |
| 0x658540 -> 0x6dc290 | ResolvePostRangedDamage | `006dc338 MOV [EAX+0xa98],ECX`, `006dc370 MOV EDX,[ECX+0xb08]`, `006dc347 CALL 0x006e2c50` (IsLightsaberEquipped), DecrementRoundLength, CheckActionLengthAtTime = Mac ResolveRangedAnimations; Mac PostRangedDamage calls GetTotalDamage/GetIsWeaponEffective | **fail: = ResolveRangedAnimations** |
| 0x6587e0 -> 0x6dc530 | IsLightsaberEquipped | `006dc57e CALL 0x006dc5c0` ResolveProjectileMissLocation + GetAttack, RET 4; Mac IsLightsaberEquipped is GetItemInSlot + IsLightsaber on 0x1150 | **fail: = ResolveRangedMiss** |
| 0x658870 -> 0x6dc5c0 | ResolveProjectileMissLocation | ClearLineOfSight, GetArea, normalize, rand, 0x380 = Mac | pass |
| 0x658d20 -> 0x6dca70 | ResolveProjectileMissImpact | AddEventDeltaTime, SetFloat, imm 0xfc0 = Mac | pass |
| 0x658e90 -> 0x6dcbe0 | SignalDamage | 6 Broadcast*Breakdown + Signal{Melee,Ranged}Damage, disp 0xc1-0x17b = Mac | pass |
| 0x659db0 -> 0x6ddb00 | SignalRangedDamage | ResolveSafeProjectile, ProjectileMiss*, SetAttackSummaryFeedbackEffectData = Mac | pass |
| 0x65a5b0 -> 0x6de2b0 | GetWeaponImpactTime | disp 0xec/0x130, GetINTEntry = Mac (also part a) | pass |
| 0x65a650 -> 0x6de350 | GetWeaponDischargeRow | disp set identical to Mac (0xe0,0xe4,0x130x3,0x1184x2) | pass |
| 0x65ab90 -> 0x6de890 | GetWeaponDischargeHand | GetWeaponDischargeRow, SubString, atoi = Mac | pass |
| 0x65ad90 -> 0x6dea90 | ResolveMeleeAttack | ResolveAttackRoll, ResolveDamage, ResolveMeleeAnimations, ResolvePostMeleeDamage, imm 0xd1-0xd3 = Mac | pass |
| 0x65b530 -> 0x6df230 | ResolveRangedHit | `006df24a`/`006df269`/`006df288 MOV [+0x59c],0/1/2` after 3 dice helpers (rand), no GetAttack; Mac ResolveRangedHit 230 B = GetAttack + magnitude, target pos 0x68/0x70 | **fail: dice helper (no Mac symbol)** |
| 0x65b5a0 -> 0x6df2a0 | ResolveMeleeSpecialAttack | Mac score 1.17, same 19 callees (also part a) | pass |
| 0x65c950 -> 0x6e0650 | SignalMeleeDamage | AddFloatyText, ServerToClientObjectId = Mac (also part a) | pass |
| 0x65ce10 -> 0x6e0b10 | ResolveAttackRoll | GetArmorClassVersus, GetCriticalHitRoll, ResolveSneakAttack, ResolveDefensiveEffects = Mac | pass |
| 0x65d300 -> 0x6e1000 | ResolveSneakAttack | GetVisibleListElement, imm 0x3c-0x45, 0x86 = Mac | pass |
| 0x65d840 -> 0x6e1540 | ResolveDamage | ResolveOnHitEffect, ResolveOnHitVisuals, BroadcastVoiceChat = Mac | pass |
| 0x65da80 -> 0x6e1780 | ResolvePostMeleeDamage | Add, CGameEffect, GetTotalDamage, SetInteger (no GetIsWeaponEffective) = Mac | pass |
| 0x65df60 -> 0x6e1c60 | ResolveDefensiveEffects | HasSpell, IsProjectileParryable, GetClassLevel, disp 0x158-0x17b = Mac | pass |
| 0x65e710 -> 0x6e2410 | ApplyOnHitSlayAlignmentGroup | `006e2440 CALL 0x006e24b0` IsProjectileParryable, `006e248b MOV byte [EAX+0x5c],0xa`, `006e2494 MOV byte [ECX+0x5c],0x8`; Mac SlayAlignmentGroup is a 5 B stub | **fail: = ResolveRangedMissToParry** |
| 0x65e7b0 -> 0x6e24b0 | IsProjectileParryable | GetForceShield, HasSpell, calls 0x65ef50 (IsLightsaberEquipped body) as Mac calls IsLightsaberEquipped | pass |
| 0x65ecb0 -> 0x6e29b0 | ResolveMoveToForceJump | TestDirectLine, CResRef, imm 0x65-0x67 = Mac | pass |
| 0x65efd0 -> 0x6e2cd0 | ResolveOnHitVisuals | GetDamage, GetINTEntry, imm 0xfb7 = Mac | pass |
| 0x65f950 -> 0x6e3650 | ResolveDamageShields | RollDice, SetNumIntegers, imm 0x3e8 = Mac | pass |
| 0x65fd40 -> 0x6e3a40 | DoCombatStep | DesiredAttackRange, RangeWeaponEquipped, asin/sin/cos = Mac (also part a) | pass |
| 0x661330 -> 0x6e5030 | CreateLinkedEffect | SetLinked etc. = Mac 3-effect overload 0x37807e (3x 0x8c) | pass |
| 0x662200 -> 0x6e5f00 | ApplyOnHitAbilityDamage | UpdateLinked, CreateLinkedEffect (0x6614d0), SetFloat, imm 0x5b/0x8e = Mac; dispatch case 6 | pass |
| 0x662e40 -> 0x6e6b40 | SendAttackSummaryFeedback | BroadcastAttackSummaryToParty, GetIsMember, disp identical = Mac | pass |
| 0x680c50 -> 0x568020 | Rest | `00568035 PUSH 0x7` + `0056803a CALL 0x006b7a00` GetCanUseSkill(7), `005680b1 PUSH 0x38` + `005680b6 CALL 0x0053f7f0` AddAction; Mac AddHealActions disp 0xec/0x1198 imm 7/0x38 (score 1.44); Mac Rest is 792 B | **fail: = AddHealActions** |
| 0x680d00 -> 0x5680d0 | AddMineActions | GetPlayerCanSetMines, 0x2c8-0x34c = Mac | pass |
| 0x656d70 -> 0x6daac0 | IsInTotalDefense | disp 0xb1c,0x10dc,0x110c = Mac (score 1.5) | pass |
| 0x656fa0 -> 0x6dacf0 | ResolveAttack | ResolveMeleeAttack/ResolveRangedAttack, SetRoundPaused = Mac | pass |
| 0x67be50 -> 0x563220 | GetDialogResref | CResRef from stats `+0x44` (GetPortrait reads +0x17a) = Mac | pass |
| 0x67bf20 -> 0x5632f0 | GetFirstName | stats name getter = Mac | pass |
| 0x681f50 -> 0x569320 | AIUpdate | 30+ callees = Mac (RunActions, HandleHP/FPRegeneration..) | pass |
| 0x694970 -> 0x57be40 | GetNearestObjects | ValidNearestObjectType, Yaw, and the B12b-proven CExoArrayList<float> Add/Insert (GOG labels LearnScroll/UseLoreOnItem) = Mac Add/Insert | pass |

**Whole-batch sweep (beyond the brief's windows).** All 150 AUTO rows were also scored by script against the Mac
symbol of the same name: disp + imm multiset Jaccard + callee-name Jaccard, with `_N` suffixes stripped. 18 rows scored
low or had a better-matching other Mac symbol, and I read each by hand. Most are compiler-shape noise (Constructor /
Destructor_2 = Mac CSWSCreature / ~CSWSCreature, LoadFollowInfo, RecomputeAmbientAnimationState, GetNearestObjects).
**2 more fail, outside the windows:**

| GOG -> Steam | GOG name | evidence | result |
|---|---|---|---|
| 0x692330 -> 0x579800 | BroadcastForcePointUsageData | `005798c1 PUSH 0x10` message id, sole caller DoSpotDetection 0x578850; Mac `BroadcastStealthDetectionData` (197 B, imm 0x10, disp 0x90 x2, 6 callees, sole caller DoSpotDetection). The Mac binary has no ForcePointUsageData symbol; Mine (0x13) is the verified sibling 0x692420 | **fail: = BroadcastStealthDetectionData** (part a guessed this) |
| 0x696e80 -> 0x57e350 | Rest_2 (GOG label `Rest`) | `0057e35c MOV ECX,[EAX+0x53c]` (creature size), GetBaseItem `0057e381 CALL 0x6d5200`, `0057e38c MOV DL,[ECX+0x1b]` (weapon size), `0057e3af MOV EAX,0xfffffff6` when the difference is outside -2..1; callers GetMeleeAttackBonus, GetAttackModifierVersus, EquipMostDamagingMeleeWeapon, 0x6e4af0; Mac `GetRelativeWeaponSize` 71 B, disp 0x1b/0x34/0x53c, imm 0xfffffff6, callers Get{Ranged,Melee}AttackBonus + GetAttackModifierVersus; Mac Rest is 792 B | **fail: = GetRelativeWeaponSize** |

Both GOG `Rest` labels in this class are wrong: 0x680C50 is AddHealActions and 0x696E80 is GetRelativeWeaponSize. The
GOG label `GetRelativeWeaponSize` sits on 0x6F5300, which is Stats::GetWeaponFocus (B12b). Real Rest is still to be
located. 0x68F220 GetUseMonkAbilities (82 B, GetItemInSlot(0x10)) cannot be compared because Mac GetUseMonkAbilities
is a 7 B stub. Its caller GetUseMonkAttackTables matches Mac, so it is not flagged.

The 2 unsampled REVIEW rows also pass: 0x656dd0 CancelAllCombatModes (ClearAllSpecialAttacks, SetCombatMode,
SetLockOrientationToObject = Mac) and 0x65a960 GetWeaponDischargeShots (GetWeaponDischargeRow + GetINTEntry; twin
string split vs HitShots as in part a).

## Part a / r rows re-read against the full rotation table

- **0x68f7d0 GetBlind -> 0x576ca0**: verdict stays `label_suspect`. The z row only refines the note. Part a guessed
  "Mac RemoveAction". The GOG callees are GetAtPos, GetNext, Remove, ~CSWSObjectActionNode and free, with disp 0x6c and
  vt 0x74 (Mac 0x78), exactly Mac `CSWSCreature::CancelAction` (284 B). B12b part z found the same. Real GetBlind = GOG
  0x69df00 -> 0x585420 (B12b).
- No other verdict changes. Every part a/r `label_suspect` stays consistent with the table. The part a/r `verified`
  rows (ResolveRangedAttack, ResolveMeleeAnimations, ResolveMeleeSpecialAttack, DoCombatStep, CalculateMaxElementalDamage,
  ResolveOnHitEffect, SavingThrowRoll, Confusion, Fear, SetAttackSummaryFeedbackEffectData, GetNearestEnemy) all have
  their Mac namesake's callee set or imm set. ResolveMeleeAnimations 0x6e0410 also calls the 0x6e0600 helper, as part a
  said.

## Full combat-block table (GOG label -> body), merged from parts a, r, z

| GOG | GOG label | body (Mac) | Steam |
|---|---|---|---|
| 0x657710 | IsInTotalDefense_2 | GetClientRangedAnimation wrapper (inlined on Mac) | 0x6DB460 |
| 0x658540 | ResolvePostRangedDamage | ResolveRangedAnimations | 0x6DC290 |
| 0x658710 | GetWeaponDischargeTime | ResolveRangedHit | 0x6DC460 |
| 0x6587E0 | IsLightsaberEquipped | ResolveRangedMiss | 0x6DC530 |
| 0x65AAE0 | (FUN_) | GetWeaponDischargeTime | 0x6DE7E0 |
| 0x65B530 | ResolveRangedHit | 0x59C dice helper (no Mac symbol) | 0x6DF230 |
| 0x65C680 | ResolveRangedMiss | anim-id predicate (no Mac symbol) | 0x6E0380 |
| 0x65C900 | ResolveRangedAnimations | attack-anim setter helper (no Mac symbol) | 0x6E0600 |
| 0x65DC30 | ApplyOnHitSleep | ResolvePostRangedDamage | 0x6E1930 |
| 0x65E710 | ApplyOnHitSlayAlignmentGroup | ResolveRangedMissToParry | 0x6E2410 |
| 0x65EF50 | ResolveRangedMissToParry | IsLightsaberEquipped | 0x6E2C50 |
| 0x6614D0 | ApplyOnHitHold | CreateLinkedEffect (2-effect overload) | 0x6E51D0 |
| 0x661950 / 0x661B60 / 0x662060 / 0x6625D0 / 0x662900 | Slow / Poison / SlayRacial / InstantDeath / Knockdown | Hold / Sleep / Stun / Poison / InstantDeath (part r dispatch table) | 0x6E5650 / 0x6E5860 / 0x6E5D60 / 0x6E62D0 / 0x6E6600 |
| 0x661E10 / 0x662690 / 0x662A30 | (FUN_) | Slow / SlayRacialGroup / Knockdown | 0x6E5B10 / 0x6E6390 / 0x6E6730 |

## Counts

| verdict | rows |
|---|---|
| verified | 1 (SetEffectSpellId) |
| label_suspect | 10 (2 part rows + 5 AUTO window audit + 2 AUTO whole-batch sweep + 1 part a note refresh, 0x68f7d0) |
| likely / rejected / unresolved | 0 |

sample_errors: 0
