# KOTOR II party AI: review of the combat decision layer (k_ai_master + includes + Mod 8 kite)

Reviewer: independent, read-only. Date: 2026-10-01.
Sources: `~/Games/KOTOR2_Mod_Workspace/mod8_kite_script/vanilla/*.nss` (line numbers are the original CRLF files' line numbers), `vanilla_all/`, `src/k_inc_generic.nss` (generated), `patch_manager_mods/ranged_ai_src/script/kite_routine.nss`.
Extra evidence used: `replay/pre_checkpoint_2da/{baseitems,feat,spells}.2da` (read through pykotor's `read_2da`) and the Steam decompile `ghidra_dumps/steam_6a522e71/decomp/68/00685960.c` (`ExecuteCommandGetObjectInShape`).

Stances: 9 Aggressive, 10 Defensive, 11 Ranged, 12 Stationary, 13 Jedi Support (Jedi -> `GN_RunJediSupportAIRoutine`, non-Jedi -> `GN_RunGrenadeAIRoutine`), 14 Remote (puppet), 16 Kite (ours). "Default AI" = `GN_RunDefaultAIRoutine`, which every stance reaches.

Confidence: **verified** = traced in source, quoted below; the player-visible effect follows directly from the code. **verified-code / effect plausible** = the code defect is certain, but what the engine does with the result (for example an invalid talent or an empty weapon config) needs an in-game check. **plausible** = depends on engine behaviour I could not confirm statically.

## Summary

| id | sev | stances | confidence | symptom |
|---|---|---|---|---|
| H1 | high | 13 (Jedi) | verified-code / effect plausible | Whenever any party member is knocked out, a Jedi Support companion without Revitalize (or without the Force points for it) clears its actions and casts nothing, every round, so it stands idle until the downed member gets up. |
| H2 | high | all (9-13, 16) | verified | AI healing only triggers when the **leader** is below 50% HP. Companions never heal themselves or each other, and never use medpacs or repair kits. |
| M1 | med | 11, 16 | verified-code / effect plausible | When hit in melee, a Ranged or Kite companion switches to weapon config 2 even when config 2 is **empty** (it only checks that config 2 is not ranged), so it holsters the blaster and fights unarmed or idles. It switches back about 2 DCR calls later. |
| M2 | med | 13 (Jedi) | verified-code / effect plausible | Jedi Support's AoE target can be an **ally**, a dead creature or an unseen one. A hostile AoE power (Storm, Wave, Insanity, Scream) is aimed at a party member who is standing among 3 enemies. There is no `GetIsEnemy` gate. |
| M3 | med | 13 (Jedi), 16 | verified | `GN_DetermineAttackTarget` (Aggressive party AI) prefers the nearest **debilitated** (stunned, held, asleep) enemy: the loop condition is inverted. Used by Jedi Support and by our kite routine. |
| M4 | med | 13 (Jedi) | verified | In `GN_GetTargetedForcePower` the parameter `nDroid` is shadowed by a local `nDroid` = 0. Organic-only powers are then chosen against droids (and swapped to a melee feat), and Droid Disable is never chosen. |
| M5 | low-med | all | verified | `GN_CheckIfPoisoned` loop `for(nCnt; nCnt > 2; nCnt++)` with nCnt=0 never runs, so party poison cure is dead code. |
| M6 | low-med | 9-12, 16 | verified-code / effect plausible | Within 20 m, the response to a "GEN_I_WAS_ATTACKED" shout about an **unseen** enemy is `ActionMoveToObject(enemy, TRUE, 2.0)` whatever the weapon or stance. On damage from an unseen enemy, the companion runs to the damager's location (k_ai_master 2006). Ranged, Stationary and Kite companions charge into melee, then the leash yanks them back. |
| M7 | low-med | 10, 11, 16 | verified | Weapon type tables miss baseitems 104 Mining_Laser and 105 Adv_Mining_Laser (ranged), and 106/107 (melee). Attackers with mining lasers count as melee (melee mark and swap, kite trigger). A companion holding one counts as "not ranged" (kite disabled, Defensive targeting uses the melee rule). |
| M8 | low-med | 11, 16 | plausible | The weapon-swap counter decrements per DCR **call**, not per round, and each swap round issues no attack. Intermittent melee hits cause ranged/melee ping-pong. With the engine's "Ranged companions don't approach" rule, a companion swapped to melee cannot reach anyone who is not adjacent. |
| M9 | low-med | 10 | verified (distance) / plausible (yo-yo) | Defensive melee filter: "enemy within 5 m of the leader" is measured from **self**, not the leader. The sticky last target bypasses the filter, and the 5 m leash (the comment says 10 m) yo-yos the companion. |
| M10 | low-med | 9-12, 16 | plausible | The sticky "last hostile target" beats both the intruder and the nearest enemy. The k_ai_master 2006 "switch to the dangerous attacker" retarget never takes effect for party members. If that last target is still seen but more than 30 m away, the companion calls `CancelCombat` and idles even though closer enemies exist. |
| K1 | med | 16 (ours) | verified-code / effect plausible | The kite routine queues `ActionAttack(oShootAt)` and repositions for a target from `GN_DetermineAttackTarget`, which has no `GetIsEnemy` or seen check. That target can be a stale last target (a neutral, or a bashed door or placeable). Vanilla Default AI gates every attack on `GetIsEnemy`. |
| K2 | low-med | 16 (ours) | plausible | There is no cap on consecutive kites. A melee enemy that keeps chasing re-enters 3.5 m every couple of DCR calls, so the companion keeps running and rarely fires. |
| K3 | low | 16 (ours) | plausible | The threat check looks only at the single nearest seen enemy. If that one is ranged (and we are above 60% HP) or dead, a melee enemy right behind it is ignored. |
| L1 | low | all (combos) | verified | Combo table defects. Sith Brutalize is never valid (a duplicated `JEDI_CRUSH` branch). Jedi Focus (Battle Meditation, 29) is unreachable (the Random range ends at 28). Sith Attack step 1 throws away its power id. Jedi Blitz step 1 uses `TalentFeat(177)`, which is the feat MASTERCRAFT_WEAPONS_II, not Force Crush. |
| L2 | low | 9-12, 16 | verified-code / effect plausible | `SW_FLAG_TARGET_FRIEND` is never set TRUE. Buffs from combos (Force Armor and Speed in Buff Attack, Fury in Sith Fury) are cast with the **enemy** as target and with no "already active" check, so active buffs get recast. |
| L3 | low | all droids | verified | Party droids never use droid utility talents (category 0x8000). Only `NPC_AISTYLE_JEDI_SUPPORT` (5, deprecated) unlocks them. Party support stance 13 sends droids to the grenade routine instead. |
| L4 | low | 13 | verified | Buff picker typo: `GetIsTalentValid(tBuff_05) && GetCreatureHasTalent(tBuff_06)`, so tier V of Crush Opposition and Inspire Followers is never chosen. |
| L5 | low | 13 (Jedi) | verified | `GN_GetAOEForcePower` uses the wrong skip flags (`nHorror` in the Insanity slot, `nKillDroid` in the Howl slot). Some rolls return no power or a skewed one, and Jedi Support falls back to the default attack. |
| L6 | low | 14 | plausible | The Remote's heal threshold is local 8, which is the same slot that k_ai_master 2005 sets to 2 on a melee hit. That leaves remote healing off, or limited to targets under 2%. The puppet also queues `ActionAttack` every DCR without `ClearAllActions` and with no dead or enemy check. |
| L7 | low | all (shouts) | verified | The `GEN_COMBAT_ACTIVE` (15) shout branch is nested inside the `nShoutIndex == 1` block, so it is dead. |
| L8 | low | 13 | plausible | Instruction budget. `GN_FindAOETarget` and `GN_FindGrenadeTarget` are O(N x M) shape scans and run on every DCR, several times a round. In crowded areas they approach the 0x20000 instruction cap. |

NPC-only observations (outside the party scope, no section): `GetNPCAIStyle(OBJECT_SELF) != SW_FLAG_BOSS_AI` (generic 687, 838) compares an AI style to the flag index 89, so the boss exemptions are dead. `nDroid == TRUE;` is a no-op statement (generic 1104, 1142). The boss branch of `GN_GetStandardJediCombo` (gensupport 586-592) loops forever if no combo 14-28 is valid. `GN_ReturnActivePartyMember` checks `oNPC2` for all three members (gensupport 3816/3825, no callers). In `GEN_TALENT_SUPRESS_FORCE`, `tUse2` is computed but `tUse` is tested (generic 2746-2747, no callers).

---

## H1. Jedi Support goes idle while a party member is knocked out

`k_inc_generic.nss:1006-1016`
```c
if (GN_CheckSeriesRevitalize() != -1)
{
    int nDown = GN_CheckIfKnockedOut();
    if ( nDown )
    {
        AurPostString( "Someone is knocked out.", 20, 25, 5.0f );
        if ( GN_TalentMasterRoutine(GEN_TALENT_REVITALIZE, GetFirstPC()) )
        {
            return TRUE;
        }
```
`k_inc_gensupport.nss:1844-1853`: `GN_CheckSeriesRevitalize()` returns `TRUE`/`FALSE`, never -1.
`k_inc_generic.nss:2512-2519` and `2725-2732`:
```c
else if (nTalentConstant == GEN_TALENT_REVITALIZE)
{
    int nID = GN_GetSeriesRevitalize();   // -1 if the creature has no Revitalize
    tUse = TalentSpell(nID);
    bValid = TRUE;                        // unconditional
    bHostile = FALSE;
...
if(bValid == TRUE && bHostile == FALSE)
{   ...
    ClearAllActions();
    ActionUseTalentOnObject(tUse, OBJECT_SELF);
    return TRUE;
```
**Why it is wrong.** The gate `!= -1` is always true. `GN_CheckIfKnockedOut()` (generic 2361-2378) is TRUE as soon as any of party indices 0-2 has HP < 1. In that case the Revitalize branch runs:
- It does not check that the creature has the power (`GetCreatureHasTalent`).
- It does not check that the creature has the Force points for it (that is what `GetCreatureHasTalent` covers, per gensupport 1199).
- It still clears the action queue, queues `TalentSpell(-1)` (or an unaffordable power) and returns TRUE.

`GN_DetermineCombatRound` (stance 13, line 442-449) then returns. The same thing happens on every DCR call.

**Symptom.** Kreia, Visas or any Jedi on "Jedi Support" stops fighting, healing and buffing as soon as someone in the party is down, and stays idle until that member revives after combat. A Jedi who has Revitalize but lacks the Force points behaves the same way.

**Confirm.** (a) In game: put a Jedi without Revitalize on Jedi Support, let a companion drop, and watch them stand still. (b) Log: the two `AurPostString` calls ("Someone is knocked out.", "Setting up to revitalize.") sit on exactly this path. A DLL hook on the AurPostString command routine that logs the string confirms the path without any script change.

**Fix (k_inc_generic).** Change the gate to `if (GN_CheckSeriesRevitalize())`. In the REVITALIZE branch, set `bValid` only when `nID != -1 && GetCreatureHasTalent(tUse)`.

## H2. AI healing only looks at the leader; companions never heal themselves or each other, and never use medpacs

`k_inc_generic.nss:2318-2345`
```c
if(GetIsObjectValid(oP0) && GetRacialType(oP0) != RACIAL_TYPE_DROID)
    fDown00 = ... oP0 ...;
if(GetIsObjectValid(oP1)&& GetRacialType(oP0) != RACIAL_TYPE_DROID)      // oP0, should be oP1
    fDown01 = ... oP1 ...;
if(GetIsObjectValid(oP2) && GetRacialType(oP0) != RACIAL_TYPE_DROID)     // oP0, should be oP2
    fDown02 = ... oP2 ...;

if(GetIsObjectValid(oP0) && !GetIsDead(oP0) && (fDown00 < 0.5 && fDown00 > 0.0))
    return oP0;
else if(GetIsObjectValid(oP1) && !GetIsDead(oP1) && (fDown00 < 0.5 && fDown00 > 0.0))   // fDown00, should be fDown01
    return oP1;
else if(GetIsObjectValid(oP2) && !GetIsDead(oP2) && (fDown00 < 0.5 && fDown00 > 0.0))   // fDown00, should be fDown02
    return oP2;
```
**Why it is wrong.** The second and third branches test the leader's ratio. They can only be reached when the first branch has already failed on that same ratio, so they are dead. The function returns `oP0` (the leader) or nothing. The droid test also always checks `oP0`, so if the leader is a droid nobody is ever "injured".

Knock-on effect in `GN_TalentMasterRoutine(GEN_TALENT_HEALING, ...)` (generic 2483-2509): medpacs and repair kits (`tSpe11_05`) are only used when `oTarget == OBJECT_SELF`. `GN_DetermineCombatRound` never runs for the leader (generic 288), so for a companion `oTarget` is never `OBJECT_SELF`, and the medpac branch is unreachable for party members.

**Symptom.**
- Companions with Heal or Cure cast it only when the leader is below 50% HP. A companion at 10% HP standing next to a healthy leader is never healed by the party's Jedi.
- Companions never use medpacs or repair kits on their own.
- If the player controls a droid, no Force heals happen at all.

**Confirm.** Let a companion fall below 50% while the leader stays above 50%, with a Heal-capable companion on Aggressive or Jedi Support: no heal is cast. Then drop the leader below 50%: the heal comes at once.

**Fix (k_inc_generic).**
- Use `oP1`/`oP2` in the race tests and `fDown01`/`fDown02` in the return branches.
- Consider returning `OBJECT_SELF` first when self is below 50%, so the medpac branch becomes reachable.
- Keep the `fDownXX > 0.0` test so knocked-out members are not chosen.

Note that the heal powers are cast on `OBJECT_SELF` (generic 2731) and are party-area powers, so only the *detection* needs fixing.

## M1. Ranged stance switches to an empty weapon config when hit in melee

`k_inc_generic.nss:3793-3807` (`GN_CheckRangedPreferred`)
```c
object oWeapon2 = GetItemInSlot(INVENTORY_SLOT_RIGHTWEAPON2);
int nType2 = 0;
if ( GetIsObjectValid(oWeapon2) )
    nType2 = GN_GetWeaponTypeFromBaseItem(GetBaseItemType(oWeapon2));
if ( nType2 != 2 )
{
    //Our second config is NOT ranged, switch to it
    ClearAllActions();
    ActionSwitchWeapons();
    return 1;
}
```
**Why it is wrong.** `nType2 == 0` covers an **empty** config 2 (and non-weapon items). The intent ("switch to melee if we have it", lines 3761-3770) needs `nType2 == 1`.

**Symptom.** A Ranged companion with no second weapon set gets hit by a melee enemy and switches to an empty set: unarmed, or no weapon drawn. On a later DCR, the "not hit recently" branch (3826-3844) sees config 2 (now the blaster) as ranged and switches back. Each switch costs the round (return 1, no attack queued).

**Ours:** `kite_routine.nss:173` step 2 calls this function unchanged, so stance 16 inherits the bug when cornered.

**Confirm.** Give a companion a blaster in config 1 and nothing in config 2, set Ranged, and let a melee enemy hit them.

**Fix.** In vanilla `GN_CheckRangedPreferred`, change the test to `nType2 == 1`. That changes stance 11 and needs k_ai_master recompiled (see "Recompile scope"). For stance 16 only, add a kite-local copy (`GN_KiteCheckWeaponConfig`) that requires `nType2 == 1` and leave vanilla alone.

## M2. Jedi Support aims hostile AoE powers at allies, corpses or unseen creatures

`k_inc_gensupport.nss:2515-2532`
```c
object oCheck = GetFirstObjectInShape(SHAPE_SPHERE, 30.0, GetLocation(OBJECT_SELF));
while(GetIsObjectValid(oCheck))
{
    nEnemy = GN_CheckEnemyGroupingOnTarget(oCheck,4.0);
    if(nEnemy > 2 && nEnemyCnt < nEnemy)
    { oFinal = oCheck; nEnemyCnt = nEnemy; }
```
`k_inc_generic.nss:1035-1047, 1071-1076`:
```c
oTarget = GN_FindAOETarget();
if(GetIsObjectValid(oTarget))
    tUse = GN_GetBossCombatMove(SW_BOSS_ATTACK_TYPE_FORCE_POWER ...);
...
if(GetIsObjectValid(oTarget) && GetIsTalentValid(tUse))
{
    ClearAllActions();
    ActionUseTalentOnObject(tUse, oTarget);
```
**Why it is wrong.** Any creature in 30 m with at least 3 live enemies within 4 m qualifies as the centre. That includes a party member fighting in a crowd, and a dead or unseen creature: there is no `GetIsEnemy`, `GetIsDead` or `GetObjectSeen` filter. Ties go to the first object in iteration order, so an ally centre is as likely as an enemy centre. The cast at 1075 has no `GetIsEnemy` gate. The Default AI does have one (generic 874/880).

**Symptom.** Kreia or Visas casts Force Storm, Wave, Insanity or Scream with Atton or the player as the target. Depending on the spell script, the power fizzles on a friendly target (wasted round and Force points), or the caster walks toward the ally. Targeting a corpse or an unseen creature wastes the round or makes the caster walk off.

**Confirm.** Put a melee companion into a group of 3 or more enemies with a Jedi Support companion that has Force Storm or Wave, and watch the cast target (target ring / combat feedback).

**Fix (gensupport).** In `GN_FindAOETarget`, require `GetIsEnemy(oCheck) && !GetIsDead(oCheck) && GetObjectSeen(oCheck)`. Optionally also call `GN_CheckFriendlyFireOnTarget(oCheck) == 0` for powers with friendly fire. This also changes the boss AI (`GN_RunBossAOEPowerRoutine`), which is desirable.

## M3. Aggressive target search prefers debilitated enemies

`k_inc_generic.nss:3425-3437` (`GN_DetermineAttackTarget`, `nPartyAI == PARTY_AISTYLE_AGGRESSIVE`)
```c
object oATarget = GetNearestCreature(... ENEMY, OBJECT_SELF, nCnt, PERCEPTION, SEEN);
oTarget = oATarget;
while(!GetIsDebilitated(oTarget) && GetIsObjectValid(oTarget))
{
    nCnt++;
    oTarget = GetNearestCreature(... nCnt ...);
}
if(!GetIsObjectValid(oTarget) && GetIsObjectValid(oATarget))
    oTarget = oATarget;
```
**Why it is wrong.** The loop skips **non**-debilitated enemies and stops at the first debilitated one. The function's own earlier checks (3411, 3416) reject debilitated intruders and last targets, so the intent is clearly the reverse. `GetPartyAIStyle()` is 0 (Aggressive) in TSL as far as the UI goes, so this branch is the normal one.

**Symptom.** Jedi Support (when there is no AoE target, generic 1052) and our kite routine (`kite_routine.nss:148`) pick the nearest **stunned, held or sleeping** enemy over the nearest active one. Jedi Support then spends Stasis or Choke on helpless enemies, or damages them (which can break sleep or horror effects), while active enemies keep attacking.

**Confirm.** Stun or hold one enemy and watch whether the Jedi Support companion's next power goes to it.

**Fix.** Change the condition to `while(GetIsDebilitated(oTarget) && GetIsObjectValid(oTarget))`. This also changes enemy NPCs that use the function (boss and Jedi Support NPC paths).

## M4. Targeted power picker ignores `nDroid`

`k_inc_gensupport.nss:3366-3369, 3384, 3434-3438`
```c
talent GN_GetTargetedForcePower(int nDroid = FALSE)
{
    talent tUse;
    int nChoke, nAfflict, nPlague, nPush, nWind, nLightning, nKill, nHorror, nWound, nStasis, nDroid, nKnock, nHowl, nCnt;
...
    if(GetHasSpell(FORCE_POWER_CHOKE) && nDroid == FALSE)
...
    if(GetHasSpell(FORCE_POWER_DROID_DISABLE) && nDroid == TRUE)
    {   nDroid = 1; nCnt++;   }
```
**Why it is wrong.** The body re-declares `nDroid`, which shadows the parameter with a fresh 0. A same-scope redeclaration would not compile, and the shipped `.ncs` is byte-identical to this source, so the body local is a new slot. Organic-only powers (Choke, Affliction, Plague, Kill, Horror, Wound, Scream) are always candidates, and Droid Disable never is.

**Symptom.** When Jedi Support (generic 1059-1066) targets a droid, it rolls an organic-only power. `GN_CheckNonDroidForcePower` (generic 1070) then swaps that for the best melee feat (`0x1104`), so a Force-focused companion runs into melee against droids instead of casting Disable Droid. Players see a Jedi Support companion that never uses its droid powers.

**Confirm.** Give a Jedi Support companion Disable Droid plus Choke and fight droids with no AoE cluster (fewer than 3 together).

**Fix (gensupport).** Rename the local (for example `nDisable`) and use it in the Disable Droid block (3434-3438, 3574-3582).

## M5. Party poison cure never runs

`k_inc_generic.nss:2255-2257`
```c
int nCnt = 0;
for(nCnt; nCnt > 2; nCnt++)
```
**Why it is wrong.** The condition is false on entry, so for party members the function always returns `OBJECT_INVALID`.

**Symptom.** Companions with Cure, Heal or Master Heal (or antidote items) never react to poison on anyone. Callers: Default AI (607-618) and Jedi Support (1019-1022).

**Confirm.** Poison a party member and watch whether a companion with Cure ever casts it.

**Fix.** Change the loop to `for(nCnt = 0; nCnt < 3; nCnt++)`.

Note that `GEN_TALENT_REMOVE_POISON` (generic 2401-2432) prefers Heal (28), then Cure (10), then Master Heal (134), and the antidote item (spell 67) only when `oTarget == OBJECT_SELF`. Once the loop runs, the powers are cast on self (area), which works.

## M6. Ranged-style companions charge unseen enemies

`k_inc_generic.nss:2035-2063` (`GN_RespondToShout`, shout 1)
```c
else if(GetIsObjectValid(oIntruder))          // reached only when !GetObjectSeen(oIntruder)
{
    ClearAllActions();
    ...
    if(GetDistanceBetween(OBJECT_SELF, oIntruder) < 20.0 && !GetObjectSeen(oIntruder))
        ActionMoveToObject(oIntruder, TRUE, 2.0);
    else
    {   if(GN_GetWeaponType(OBJECT_SELF) == 1) ActionMoveToObject(oIntruder, TRUE, 4.0);
        else                                   ActionMoveToObject(oIntruder, TRUE, 15.0); }
```
`k_ai_master.nss:759-766` (2006, on damage):
```c
if(!GN_GetIsFighting(OBJECT_SELF))
{
    object oTarget = GetLastDamager();
    if(!GetObjectSeen(oTarget) && GetArea(OBJECT_SELF) == GetArea(oTarget))
    {
        ActionMoveToLocation(GetLocation(oTarget), TRUE);
        ActionDoCommand(GN_DetermineCombatRound());
```
**Why it is wrong.** In the shout path, `!GetObjectSeen` is already guaranteed by the else-if, so every unseen intruder within 20 m gets a 2 m approach. The weapon-aware 4 m / 15 m ranges only apply beyond 20 m. The comment at 2047-2048 says this block exists to stop blaster users from running up to enemies. Neither path looks at the stance.

**Symptom.** On Ranged, Stationary or Kite, a companion whose ally was shot by an unseen enemy (or who was shot itself) runs to within 2 m of the shooter, or to its exact position. The next DCR's leash (`GN_CheckRangeFromLeader`, 10 m) pulls it back, so it yo-yos.

**Confirm.** Get the party shot from out of sight (around a corner) with a companion on Ranged.

**Fix.**
- Shout path: use the weapon-based distance for the < 20 m case too.
- Both paths: skip the approach, or cap it at the leash, when `GetNPCAIStyle(OBJECT_SELF)` is 11, 12 or 16.
- Where the code lives: the 2006 path is in k_ai_master itself; the shout path is in k_inc_generic.

## M7. Weapon type tables miss TSL base items 104-107

`k_inc_gensupport.nss:2252-2295` (`GN_GetWeaponTypeFromBaseItem`; `GN_GetWeaponType` 2153-2247 has the same list) covers baseitems 0-24 and 77-80 plus 93. `baseitems.2da` also has:
- `104 Mining_Laser` (ranged=1, maxrange 23)
- `105 Adv_Mining_Laser` (ranged=1, maxrange 28)
- `106 Lightsaber_No_Sound` (melee)
- `107 DM_ElectroStaff` (melee)

`k_ai_master.nss:685-699`:
```c
nWeaponType = GN_GetWeaponTypeFromBaseItem(GetBaseItemType(oWeapon));
if ( nWeaponType == 1 || nWeaponType == 0)
    SetLocalNumber( OBJECT_SELF, SW_NUMBER_HEALERAI_THRESHOLD, 2);
```
**Symptom.**
- Being shot with a mining laser (Peragus) counts as a melee hit, which triggers the M1/M8 swap on Ranged and Kite.
- The kite routine (`kite_routine.nss:158`) treats a mining-laser enemy within 3.5 m as a melee threat.
- A companion holding a mining laser is "not ranged": the kite logic is skipped, Defensive uses the melee target rule (generic 657-682), and `GN_GetSeriesRangedFeat` hands out melee feats.

**Confirm.** On Peragus, put a companion on Ranged with a melee weapon in config 2, and let mining-laser enemies shoot them.

**Fix (gensupport).** Add 104/105 to the ranged list and 106/107 to the melee list in both functions. Our kite routine uses the same helper, so it picks up the fix.

## M8. Weapon-swap counter is per DCR call, and swap rounds do nothing

`k_inc_generic.nss:3773-3778` decrements `SW_NUMBER_HEALERAI_THRESHOLD` on every `GN_CheckRangedPreferred` call. k_ai_master 2005 sets it to 2 on every melee or unarmed hit. DCR runs on 2003 (round end), 2005 and 2006, plus shouts, so the "2 rounds" is often less than one round. A swap returns 1, so `GN_RunRangedSupportAIRoutine` returns without `GN_RunDefaultAIRoutine` (3737-3740) and no attack is issued that call. The engine's no-approach rule for stances 11/12, and for 16 via E-H3 (doc 13 S1), means that after a swap to melee the companion only fights adjacent enemies.

**Symptom.** When melee hits come intermittently (an attacker that alternates targets, or a knockdown), the companion flips blaster, melee, blaster, losing a round each time. Ours: the kite routine resets the counter to 0 after every kite (`kite_routine.nss:120`), which speeds up the switch back.

**Confirm.** Log `GetLocalNumber(self, 8)` and the equipped base item per DCR. Alternatively, count `ActionSwitchWeapons` events in the DLL.

**Fix.** Count rounds only on the 2003 event: pass a flag, or decrement in k_ai_master 2003 instead of inside `GN_CheckRangedPreferred`. After a swap to melee, also queue `ActionAttack(GetLastAttacker())` if that attacker is adjacent.

## M9. Defensive melee filter measures the wrong distance

`k_inc_generic.nss:670-681`
```c
//If we have melee equipped, we only attack creatures that are within 5.0 meters of the player character
oClose = GetNearestCreature(CREATURE_TYPE_REPUTATION, REPUTATION_TYPE_ENEMY, oLeader,1, CREATURE_TYPE_PERCEPTION, PERCEPTION_SEEN);
if ( GetIsObjectValid(oClose) && GetDistanceToObject(oClose) > 5.0f )   // distance from OBJECT_SELF
```
Also the leash: `k_inc_generic.nss:393-398`, where the comment says 10 m and the code uses `GN_CheckRangeFromLeader(5.0f)`. And the sticky last target at 708 is accepted before this filter is consulted.

**Symptom.** A Defensive melee companion ignores an enemy that is hitting the leader from the far side (more than 5 m from the companion). Meanwhile it keeps chasing its previous target past the 5 m leash, and every DCR pulls it back to 2.5 m from the leader, so it yo-yos and rarely lands a hit.

**Fix.** Use `GetDistanceBetween(oClose, oLeader)`. Apply the same leader-distance filter to the sticky `oTarget` when the stance is Defensive.

## M10. The sticky last target beats retargeting, and the 30 m drop-out idles the companion

`k_inc_generic.nss:708-711` keeps `tPR.oLastTarget` whenever it is alive and seen, ahead of `oIntruder` and `oClose`. `837-844`:
```c
float fDistance = GetDistanceToObject(oTarget);
if ( fDistance > 30.0f && GetNPCAIStyle(OBJECT_SELF) != SW_FLAG_BOSS_AI )
{   oTarget = OBJECT_INVALID;  CancelCombat(OBJECT_SELF); }
```
**Why it is wrong.**
- k_ai_master 2006 (779-784) calls `GN_DetermineCombatRound(oAttacker)` to switch to a dangerous attacker, but for party members the intruder is ignored while the last target lives.
- If the last target flees but stays visible beyond 30 m, combat is cancelled even when nearer enemies exist. Default AI returns FALSE, and the post-DCR checks do nothing while enemies are visible.

The 708 comment ("party members only attack what you want") suggests the stickiness is intended for player-chosen targets. It also applies to targets the AI picked itself.

**Symptom.** A companion keeps hitting the same weak target while an elite chews on it. When an enemy flees across a large map, the companion stands idle in the middle of the fight.

**Fix.** When `oIntruder` is valid, seen and an enemy, and the call came from the "dangerous attacker" path, let it override the sticky target. In the 30 m branch, fall back to `oClose` instead of cancelling combat.

## K1. Kite routine can attack or reposition for a non-enemy target

`kite_routine.nss:148, 162-166, 179-187`
```c
object oTarget = GN_DetermineAttackTarget(oIntruder);
...
object oShootAt = GetIsObjectValid(oTarget) ? oTarget : oThreat;
if (GN_KiteFindSpot(1, oThreat, oShootAt, oLeader))
{   GN_KiteMoveThenAttack(oShootAt, 2);  ...   // ClearAllActions; move; ActionAttack(oShootAt)
...
if (nRanged && nCool == 0 && GetIsObjectValid(oTarget) && !GetIsDead(oTarget) && !GN_KiteHasShot(...))
```
**Why it is wrong.** `GN_DetermineAttackTarget` returns `oIntruder`, or else `GetLastHostileTarget()`, with no `GetIsEnemy` or `GetObjectSeen` test (generic 3411-3420). It also has the M3 inversion. The last hostile target can be a creature that is no longer hostile (plot surrender, faction change), or possibly a door or placeable the companion bashed. Vanilla Default AI never attacks without `GetIsEnemy(oTarget)` (generic 874-885).

**Symptom.** A stance-16 companion kites and then queues `ActionAttack` on a neutral NPC or a container. Or it walks to "get a clear shot" at a target that is not an enemy, or that it cannot see.

**Confirm.** Fight next to a creature that turns neutral mid-fight, or bash a footlocker with the companion and then start a fight.

**Fix (ours, kite_routine.nss only).** After line 148, add `if (GetIsObjectValid(oTarget) && (!GetIsEnemy(oTarget) || GetIsDead(oTarget))) oTarget = OBJECT_INVALID;`. Optionally fall back to the nearest seen enemy. Also invert M3 locally if the vanilla fix is not applied.

## K2. No cap on consecutive kites

`kite_routine.nss:135-170`: after cooldown 2 (two DCR calls), any melee enemy within 3.5 m triggers another move. Movement speeds are equal, so a chaser re-closes almost at once. The `ActionAttack` queued after each move rarely completes before the next round-end DCR replans.

**Symptom.** A companion runs in circles near the leader and rarely shoots.

**Confirm.** One fast melee enemy against one stance-16 companion; count shots against kites.

**Fix.** Keep a second local (for example 21, also unused by vanilla) counting consecutive kites, and after 2 kites in a row fall through to steps 2 and 4. Or require the threat to be targeting us (`GetAttackTarget(oThreat) == OBJECT_SELF`).

## K3. The threat test uses only the nearest seen enemy

`kite_routine.nss:153-160`: `GetNearestCreature(... ENEMY, OBJECT_SELF, 1, PERCEPTION, SEEN)`. If the nearest enemy is a ranged one (and we are above 60% HP) or a corpse (`!GetIsDead` rejects it), a melee enemy at 2-3 m behind it is ignored. This is low impact.

**Fix.** Loop nth = 1..3 while within 3.5 m.

## L1. Combo table defects (k_inc_gensupport)

- `2022-2035`: `SW_COMBO_JEDI_CRUSH` is tested twice. The second block (Choke + Drain + Push) was meant for `SW_COMBO_SITH_BRUTALIZE` (22), which is therefore never valid.
- `575-584`: `nRand = Random(25)+1` gives 1..25, and the `+3` maps 10..25 to 13..28, so `SW_COMBO_JEDI_FOCUS` (29) is never chosen. Combos 9-12 are also never chosen. `nRand >= 10 || nRand <= 26` is always true.
- `738-742`: Sith Attack step 1 calls `GN_GetSeriesScream();` and discards the result, so `nID` stays -1 and the step is a plain attack.
- `931-932`: Jedi Blitz step 1 sets `nID = FORCE_POWER_FORCE_CRUSH; nFeat = TRUE;`, which produces `TalentFeat(177)`. feat.2da row 177 is `MASTERCRAFT_WEAPONS_II`, so the step falls back to a plain attack (or uses a passive feat).

**Symptom.** Jedi companions never use those combo steps. Low impact.

**Fix.** Each is a one-line change: `SW_COMBO_SITH_BRUTALIZE`; `Random(26)+1`; `nID = GN_GetSeriesScream();`; `nFeat = FALSE`.

## L2. Combo buffs target the enemy and are recast while active

`k_inc_gensupport.nss:727` sets `SW_FLAG_TARGET_FRIEND` (84) to FALSE every call, and nothing in the 638 scripts ever sets it TRUE (grep). So `k_inc_generic.nss:627-638` always takes the enemy-target path. In Buff Attack (754-770) and Sith Fury (979-994), Force Armor, Speed and Fury (spells.2da range `P`) are returned as `tUse` and used with `ActionUseTalentOnObject(tUse, oTarget)` against the enemy (generic 874-878). There is no `GetHasSpellEffect` test, unlike the Jedi Support buff path (generic 2655-2671). `nBuff` is set but never read.

**Symptom (plausible).** Companions recast Armor or Speed that is already running and waste Force points, and may walk toward the enemy to "cast" a personal power.

**Fix.** In `GN_GetNextTalentInCombo`, when `nBuff` is TRUE: return an invalid talent if `GetHasSpellEffect(nID)`, otherwise set `SW_FLAG_TARGET_FRIEND`. Default AI then targets self (630-637).

## L3. Party droids never use droid utilities

`k_inc_gensupport.nss:621`: `(IsObjectPartyMember(OBJECT_SELF) && GetNPCAIStyle(OBJECT_SELF) == NPC_AISTYLE_JEDI_SUPPORT)` checks style 5, which is deprecated (nwscript 1593). The party "Jedi Support" stance is 13. `k_inc_generic.nss:2845-2849` also excludes party members from the utility path, and non-Jedi on stance 13 go to the grenade routine, which only knows grenades and wrist-launcher rockets.

**Symptom.** T3-M4, HK-47 and G0-T0 never fire flamethrowers, shield disruptors or stun rays on their own.

**Fix.** Use `NPC_AISTYLE_PARTY_SUPPORT` (13) at 621 and/or add a utility attempt (`GetCreatureTalentRandom(0x8000)`, already filtered by `GN_CheckDroidUtilityUsage`) to the stance-13 droid path.

## L4. Buff tier V typo

`k_inc_generic.nss:2623-2624`: `GetIsTalentValid(tBuff_05) && GetCreatureHasTalent(tBuff_06)`. Crush Opposition V and Inspire Followers V are never chosen; the picker drops to IV. This only matters for the Exile when another character is controlled (trees 6-8 need `GetFirstPC() == OBJECT_SELF`).

**Fix.** Change it to `GetCreatureHasTalent(tBuff_05)`.

## L5. AoE power picker skip flags

`k_inc_gensupport.nss:3235` reads `if(nHorror == 1 && nRoll > 1)` in the Insanity slot, and `3310` reads `if(nKillDroid == 1 && nRoll == 1)` for Sonic Howl. The roll walk is therefore off by one for some power sets. Some rolls return an invalid talent, which makes Jedi Support fall back to the default attack. Others give two powers to one slot.

**Fix.** Use `nInsanity` and `nHowl`.

## L6. Remote (puppet) local 8 and its action queue

`k_inc_generic.nss:3932` reads the heal threshold from local 8 (`SW_NUMBER_HEALERAI_THRESHOLD`). Nothing sets it for the remote (`k_oei_hench_inc.nss:420-455`), so it is 0 and healing is off, unless k_ai_master 2005 sets it to 2 after a melee hit. In that case the remote heals droids only below 2%. `4015-4022`: `ActionAttack(GetLastHostileTarget(oOwner))` runs on every DCR without `ClearAllActions` and with no dead or enemy check. During a long fight that piles up queued attacks, and the remote can attack something the owner bashed. This is plausible and low.

## L7. Dead shout branch

`k_inc_generic.nss:2087`: `else if(nShoutIndex == 15 && ...)` is attached to the `if` at 2022, inside `if(nShoutIndex == 1 ...)` (2010), so it can never be true. Party members do not answer "GEN_COMBAT_ACTIVE" from friendly NPCs. Low impact.

## L8. Instruction budget of the shape scans

`GN_FindAOETarget` scans every creature within 30 m. It has no seen or dead gate, so neutrals and creatures behind walls count too, and it runs a 4 m inner scan for each one. `GN_FindGrenadeTarget` scans 40 m and runs two inner scans for each seen creature. Rough cost: about 35 instructions per inner object. With 80 creatures in range and about 20 in each 4 m sphere, the grenade scan alone comes to roughly 110k instructions, against a cap of 131,072. Jedi Support calls `GN_FindAOETarget` on every DCR.

Mitigation found in the engine: the area-global shape cursor (see "checked") makes the outer loop skip neighbours, which cuts N in clusters. This is plausible and only matters in very crowded areas.

**Confirm.** The engine reports an aborted script run; the DLL could log VM aborts.

---

## Recompile scope

- Fixes in `k_inc_generic.nss` or `k_inc_gensupport.nss` take effect in party combat once **k_ai_master** is rebuilt. Our `build_kite_script.py` already does that and proves the toolchain.
- The cleanest route is more asserted-anchor substitutions in `build_kite_script.py`. They then ship only in our k_ai_master, and the vanilla `.nss` files stay untouched.
- 52 vanilla scripts include `k_inc_generic`; `k_fmine_hbeat` and `k_fmine_die` include `k_inc_gensupport` directly.
- Scripts that call `GN_DetermineCombatRound` live (and so run the decision layer for anyone, party members included): `a_atkonend`, `a_force_combat`, `k_combat_rnd`, `k_def_rependd`, plus the `k_def_*` NPC event scripts.
- For party behaviour, k_ai_master plus `a_atkonend`, `a_force_combat` and `k_combat_rnd` is enough. The rest are NPC spawn, userdef or zone scripts (full list: `grep -l '#include "k_inc_generic"' vanilla_all/*.nss`).
- Note that **module-embedded** copies of these scripts (inside `.mod`/`.rim`) keep the old include until they are recompiled. That only matters for NPCs.
- k_ai_master-only fixes: M6's 2006 path and the 2005 counter in M8.
- Kite-only fixes (K1-K3, and the M1 local variant): `kite_routine.nss`.

## Coverage

- **Read fully:**
  - `k_ai_master.nss`, all cases.
  - `k_inc_generic.nss`: lines 1-1840 (DCR, Default AI, Aid, Grenade, Jedi Support, boss, healer, skirmish, Malak), 1843-2420 (shields, resistances, shouts, listen patterns, poison, injured, KO, talent master, last-round data), 2833-3135 (combo move and talent checks), 3400-3600 (attack target, talent code), 3598-4026 (leash, ranged support, ranged preferred, IsJedi, puppet).
  - `k_inc_gensupport.nss`: lines 1-2160 (constants, combos, series, combo validity), 2252-2400 (weapon type, equip), 2413-3870 (friendly fire, grenade and AoE targets, grenade talent, boss move, AoE and targeted powers, party member pickers).
  - `k_inc_switch.nss`.
  - `kite_routine.nss`, and the dispatch diff of `src/k_inc_generic.nss` against vanilla.
  - The k_hen_* event scripts' live bodies.
- **Skimmed:**
  - `k_inc_generic` turtle and monster-powers routines (1685-1830, NPC only) and ambient, commoner and droid-deactivation code (3136-3390).
  - `k_inc_walkways` (flag constants and spawn-condition getters only).
  - `k_sup_grenade` impact radius and filter.
  - `k_oei_hench_inc` remote spawn.
  - doc 13 (stance and engine rules).
  - Ghidra `0x685960`.
- **Not covered:**
  - `k_inc_utility`, `k_inc_drop`, `k_inc_debug` bodies (not part of the decision layer).
  - `k_inc_force` spell impact scripts. Whether a hostile power cast on an ally fizzles (M2) or a personal buff cast "on" an enemy applies to self (L2) depends on them and on the engine.
  - The `.utc` event script assignment for each companion, for example whether the Remote's OnAttacked is `k_hen_attacked01` (L6).
  - Engine semantics of `ActionUseTalentOnObject(invalid talent)`, `ActionSwitchWeapons` with an empty config, and `GetLastHostileTarget` for placeables (H1, M1, K1 effects).
  - Dialog-parameter scripts (`a_localn_*`) that could write local 20 at runtime (see EXCERPTS section 8).

## Checked, no bug

- **Local number 20 (ours).** Vanilla literal indices are 0, 8, 9, 11-16 and 29. `SW_NUMBER_*` are 3-11 and `WALKWAYS_*` are 0-2. No vanilla `.nss` touches 20. The value range is 0-255 (nwscript 5162-5164), and the cooldown stays in 0-2. The nwscript comment "index 12-28" is stale (29 is used by vanilla).
- **Local booleans.** The `SW_FLAG_*` live set (generic 98-133, walkways 34-55, `TARGET_FRIEND` 84) has no collisions between flags that are in use. `SW_FLAG_WAYPOINT_PAUSE_SHORT` equals the deprecated `WALK_STOP` (102), which is intentional.
- **Grenade friendly fire.** `GN_CheckFriendlyFireOnTarget` uses 4.0 m and `k_sup_grenade` blasts 4.0 m around the target location. The impact hits everything that is not neutral, so allies are protected exactly at the radius, with no margin if they move during the throw. The thrower is excluded (counted as a friend). Party grenade use requires at least 2 enemies and 0 friends.
- **Nested shape iteration.** `ExecuteCommandGetObjectInShape` (Steam `0x685960`) keeps the Next cursor in the **area** (`area+0x1a0`), and Next re-applies the caller's shape. The inner 4 m loops therefore make the outer loop skip some nearby candidates, but each evaluated candidate's friend and enemy counts are complete. That is less optimal but still safe.
- **DCR and player orders.** `GN_DetermineCombatRound` gates the whole decision layer on `!GetUserActionsPending()` (generic 286), so its `ClearAllActions` calls do not wipe queued player orders. The heartbeat 2001 follow-leader `ClearAllActions` only fires with no seen enemy and no attack, spell or move target.
- **Combo selection loops terminate for party members.** Jedi: values 1-9 return without validation (9/25 per iteration). Droid and NPC: `RANGED_CAUTIOUS` and `MELEE_CAUTIOUS` are always valid.
- **`GN_EquipAppropriateWeapon`** is a no-op for party members (both branches require `!IsObjectPartyMember`, gensupport 2318/2363). **`GN_ActivateForceField` and `GN_ActivateResistances`** exclude party members by design (comment at generic 1836).
- **Stance 16 dispatch.** It runs the leash first, then `GN_RunKiteAIRoutine`, then `return`, with no fall-through to post-DCR. Other DCR callers that are not rebuilt run the default branch for 16 (documented).
- **Kite engine-call argument orders** match the nwscript prototypes: `HasLineOfSight` (vSource, vTarget, oSource, oTarget), `GetRandomDestination`, `Location`, `ActionMoveToLocation`, `GetNearestCreature` (criteria pairs), `GetItemInSlot`, `GetDistanceBetween`.
- **`GN_CheckRangeFromLeader` and `GN_CheckRangeFromOwner`:** dead, leader and moving checks are correct.
- **`GN_GetComboMove`** combo index wrap: it is reset at 4 or 0, and the 0-255 storage is fine.
- **DelayCommand and ActionDoCommand on party paths.** Only `ActionDoCommand(GN_DetermineCombatRound())` on `OBJECT_SELF` (k_ai_master 765), with no cross-object targets. The `DelayCommand(DCR)` calls (1002, healer AI) are NPC-only.
- **Debilitated and dead filtering of intruder and last target** in `GN_DetermineAttackTarget` (3411, 3416) is correct; only the Aggressive fallback is inverted (M3).

Debug tip for in-game confirmation: `GN_MyPrintString` only prints when `!ShipBuild()` (gensupport 3898-3904). A DLL hook that makes `ShipBuild` return 0 for k_ai_master, or that logs the AurPostString command, turns on the full vanilla "GENERIC DEBUG" trace without any script change.
