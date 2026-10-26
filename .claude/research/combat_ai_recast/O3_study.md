# O3 study: party AoE centre and an "already affected" filter (read-only)

Sources: vanilla `mod8_kite_script/vanilla/k_inc_gensupport.nss` (gensupport), `k_inc_generic.nss` (generic), `nwscript.nss`; our `ranged_ai_src/script/party_ai_fixes.nss` (helper).

## 1. Who picks the AoE centre, and how it scores

- `GN_FindAOETarget()` at gensupport:2515-2532. Key lines:
  - `2520 object oCheck = GetFirstObjectInShape(SHAPE_SPHERE, 30.0, GetLocation(OBJECT_SELF));`
  - `2523 nEnemy = GN_CheckEnemyGroupingOnTarget(oCheck,4.0);`
  - `2524 if(nEnemy > 2 && nEnemyCnt < nEnemy)`
  - `2526-2527 oFinal = oCheck; nEnemyCnt = nEnemy;`
- Score = plain count of enemies in a fixed 4.0 m sphere around each candidate (`GN_CheckEnemyGroupingOnTarget`, gensupport:2443-2457: `GetIsEnemy && !GetIsDead`). Candidates = creatures within 30 m of self (default filter `OBJECT_TYPE_CREATURE`, nwscript.nss:2704). Threshold is more than 2. Ties: first found wins (strict `<`).
- No seen check, no candidate dead check, no friendly-fire check (`GN_CheckFriendlyFireOnTarget`, gensupport:2413, is used only by the grenade finder at 2488), no affected-by-power check. The 4 m radius is not the power's real radius.
- The centre is chosen BEFORE the power. `GN_GetAOEForcePower(int nDroid)` (gensupport:3122) takes no target, has no FP or cooldown check, and skips only `GetLastForcePowerUsed` (3135).

## 2. Party code paths and who uses it

- Dispatch: `GN_DetermineCombatRound` (generic:261). `NPC_AISTYLE_PARTY_SUPPORT` (13) at generic:434-441: `GN_IsJedi` -> `GN_RunJediSupportAIRoutine` (generic:976); else `GN_RunGrenadeAIRoutine`. Style 13 has no leader-range gate (that gate is style 5 only, generic:370).
- Jedi Support body: heal, poison, revitalize, self-buff run first (generic:1000-1034). Then `oTarget = GN_FindAOETarget();` (generic:1035). If a centre exists: `GN_GetBossCombatMove(SW_BOSS_ATTACK_TYPE_FORCE_POWER[,TRUE])` (generic:1041/1045) -> `GN_GetAOEForcePower` (gensupport:3039).
- If no centre: `GN_DetermineAttackTarget`, then `GN_GetBossCombatMove(SW_BOSS_ATTACK_TYPE_NPC)` (generic:1061/1065). At gensupport:3048-3056 that is a 50% roll for targeted, else an AOE power aimed at the single attack target. So AOE also hits one target.
- Ranged 11 and Stationary 12 go to `GN_RunRangedSupportAIRoutine` (generic:3706) -> default AI. Defense 10, Aggressive 9 -> default AI. None of them reach the AoE picker.
- Boss AoE (`GN_RunBossAOEPowerRoutine`, generic:1131) runs only with `SW_FLAG_BOSS_AI` (generic:328-330). Not checked against party templates.
- Answer: party members use it only when Jedi and on style 13.

## 3. Where the filter goes, and cost

- Centre filter: inside the inner count at gensupport:2523. It needs the power id, so `GN_FindAOETarget` gains an `nSpell` parameter and generic:1035 moves after the power pick.
- Single-target AoE filter: `GN_GetAOEForcePower` needs an `oTarget` parameter and an exclusion check at its flag block (gensupport:3139-3205).
- Helper `GN_PartyFamilyActive` (party_ai_fixes.nss:176-200) is not enough as-is: `GN_PartyPowerFamily` (:94) returns 0 for Horror 30, Insanity 31, Force Storm 25, Scream 159-161, Sleep 44 (Mass Stasis). Those need new family ids or a direct spell-id match.
- Cost per round, N enemies in a 5-enemy clump (effect-walk length L is unmeasured; assume 5-20):
  - Vanilla: about C^2 shape steps (C = candidates), zero effect walks.
  - Option A inline (walk every enemy in every candidate's 4 m sphere): about 25-40 walks, times L.
  - Option A cached (one walk per enemy per round): N = 5 walks.
  - Option B (chosen centre only): k = 3-5 walks per attempt.
- Engine note: `GetFirstEffect/GetNextEffect` (nwscript:2476-2479) are per-object, separate from the shape iterator. Vanilla already nests shape scans (2523 inside 2520). Nested-iterator behaviour is not verified.

## 4. What happens today when every enemy in radius is already affected

- Nothing checks the effect. `GN_FindAOETarget` returns the same max-count centre (gensupport:2524-2527). The power is rolled from applicable flags. The only guard is `nLastForcePower != X`, so the same power is blocked back-to-back but can recast after any other power.
- Result: a recast on the same centre each round the power is not the last one used. No FP or cooldown check in the picker.
- Roll-chain bugs (gensupport): Insanity's decrement tests `nHorror` (:3235); Sonic Howl's select tests `nKillDroid` (:3310). Simulated (scratchpad `o3_roll_sim.py`, faithful to :3212-3320):
  - Insanity + Storm + Wave, Horror off: Insanity 1/3, Wave 1/3, null 1/3, Storm never.
  - Horror + Storm + Wave, Insanity off (just cast): Horror 1/3, Storm 2/3, Wave never.
- A null roll falls through to `GN_RunDefaultAIRoutine` (generic:1071 onward).
- Whether the engine refuses re-applying an effect to an affected target is not verifiable from scripts (OPEN).

## 5. Options and recommendation

- Scope: Plague and Crush Opposition are not AoE picks. Plague is targeted-only (gensupport:3394, :3496), and our family 20 already filters it on the targeted path (F2b). Crush Opposition 144-149 appears in no picker here (only `UT_IsDarkSidePower`, utility:1233-1238). AoE picks in scope: Horror, Insanity, Force Storm, Mass Stasis (Sleep 44, gensupport:3169), Force Scream.
- Label issue to fix: party_ai_fixes.nss:161-162 "Stun / Stasis" uses 29, which is `FORCE_POWER_HOLD` (nwscript:795). It omits Sleep 44, while vanilla `GN_CheckSeriesHold` (gensupport:1673-1680) includes it. Family 22 (:156) holds only 16; vanilla `GN_CheckSeriesFear` (:1665-1672) includes 30 and 31.
- A. Centre score = unaffected enemies within 4 m, power picked first. Pro: actually prefers unaffected groups. Con: signature change, reorder of generic:1035, about 25-40 walks per round unless cached.
- B. Keep the vanilla centre. After the power is rolled, count unaffected enemies within 4 m of the centre. If fewer than 2, drop that power, re-roll, or fall through to default. Pro: small diff and bounded cost (k walks per try); removes the wasted recast. Con: cannot move the centre, so a fully affected centre means a default attack.
- C. Leave as is. Pro: zero cost, zero risk. Con: recasts on the same centre; FP and rounds wasted.
- Recommendation: B, limited to lasting control powers (Horror 30, Insanity 31, Mass Stasis 44), K = 2 unaffected. Leave damage-type area powers (Force Storm 25, Scream 159-161) unfiltered. Fix the roll-chain bugs at :3235 and :3310 in the same change. Move to A only if a probe shows the vanilla centre is mostly affected.
- OPEN: which of these powers leave a lasting, spell-tagged effect (`GetEffectSpellId`) needs a live probe; the scripts do not say.

## 6. Decision

- 2026-10-25: owner chose B as recommended (Horror 30, Insanity 31, Mass Stasis 44 need >= 2 unaffected enemies within 4 m of the vanilla centre; Storm 25 / Scream 159-161 unfiltered; fix gensupport :3235 and :3310). Party members only. Implementation WI added from WI-140.
