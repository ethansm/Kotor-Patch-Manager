# KOTOR / TSL combat AI: community research (web + one local cross-check)

Legend: VERIFIED = I read it in the source at the URL. UNVERIFIED = secondhand or from a search-result summary only.
Sections marked LOCAL were read from the user's own extracted vanilla TSL scripts, not the web (see section 2). They are included because the web sources do not host TSL's k_inc_generic.

## 0. Bottom line

1. No published mod does kiting or lateral repositioning for party ranged members. The closest things are:
   - GearHead's Improved AI mods (feat and force power selection only).
   - blinkyzero's unfinished K1 Enemy Enhancer.
   - Vanilla TSL's own "Ranged" party stance, which is only a leash plus a weapon swap (section 2).
2. TSL vanilla already contains two relevant building blocks:
   - `HasLineOfSight(vector, vector)` (function 820) and `GetRandomDestination`, used by an OEI random-reposition-for-LOS routine that is restricted to non-party droids (section 2.4).
   - `NPC_AISTYLE_PARTY_RANGED = 11` with `GN_RunRangedSupportAIRoutine`, which never moves the creature away from melee enemies.
3. Party AI runs through `k_ai_master.ncs` events 2001 to 2014, then `GN_DetermineCombatRound()` in `k_inc_generic`. A replacement `k_ai_master.ncs` in Override is the standard, community-proven hook. Compatibility issue: every other AI mod also replaces `k_ai_master.ncs`, so mods conflict.
4. Engine level, nothing public documents the vanilla approach or attack-range logic. Public engine reimplementations (reone, KotOR.js) use `baseitems.2da` `maxattackrange` (reone) or hardcode 2.0 and 15.0 (KotOR.js). Their `MoveAwayFromObject` is a stub (reone) or absent (KotOR.js). They are not evidence of vanilla behaviour.

## 1. Existing AI mods

| Mod | URL | Changes | TSLRCM compat | Source |
|---|---|---|---|---|
| KotOR2 - Improved AI 1.3.4 (GearHead) | https://deadlystream.com/files/file/2417-kotor2-improved-ai/ and topic https://deadlystream.com/topic/11007-modkotor2-improved-ai/ | Enemy force users use saber styles, forms and all powers. Buffs are cast more often. Shields reactivate after 200 s. NPCs use the highest-level feat or power. Team-wide healing. T3-M4 auto-uses Shock Arm and Renewable Shield. Bao-Dur uses Shield Breaker. Primarily replaces `k_ai_master.ncs`. | Yes (VERIFIED on file page) | Optional source included (VERIFIED on the topic page) |
| KotOR1 - Improved AI (GearHead) | https://deadlystream.com/files/file/2328-kotor1-improved-ai/ and https://deadlystream.com/topic/10774-modkotor1-improved-ai/ | Companions and NPCs use blaster feats (Master Rapid Shot / Flurry). Droids excluded because of buggy animations. Enemy force users use Force Breach and Lightsaber Throw. Author says companions still do not avoid mines or auto-buff. | K1R compatible | Topic does not mention source. A search snippet said a "Source" folder exists (UNVERIFIED). |
| Finesse: Melee Weapons and misc. fixes (K1) | https://deadlystream.com/files/file/2464-finesse-melee-weapons-and-misc-fixes/ | Replaces `k_ai_master.ncs` for the finesse ticker (0.5 s update) and restores the repeating-blaster extra attack. | Incompatible with other `k_ai_master` replacers, including some versions of K1 Improved AI (VERIFIED) | not stated |
| KotOR Enemy Enhancer WIP (blinkyzero, K1, abandoned) | https://deadlystream.com/topic/4645-wip-kotor1-enemy-enhancer/ | Enemies focus weak or healer party members. Ranged attackers use feats. Multi-use shields. Includes a "Passive" script so companions do nothing in combat. | K1 only | never released |
| KOTOR Party AI Restorer (DarthParametric) | https://github.com/DarthParametric/KOTOR_Party_AI_Restorer | Armband that checks and re-enables party AI flags after cutscenes leave AI disabled. Not a behaviour change. | n/a | GitHub, has a SOURCE folder |
| "AI off" k_ai_master.ncs | https://deadlystream.com/topic/7437-help-making-my-first-mod-turning-off-henchman-ai/ | Remove or disable the henchman handlers 2002, 2003, 2005 and 2006 in `k_ai_master`. DarthParametric recommends KTool BIFs -> scripts.bif -> Script, Source for vanilla source, plus DeNCS. | n/a | n/a |
| Companion tweaks / Companion Fix | https://deadlystream.com/files/file/1759-kotor-companion-tweaks/ and https://deadlystream.com/files/file/2404-kotor2-companion-fix/ | Stats only (UNVERIFIED, search-result summary) | | |

Community statements on feasibility:
- Deadlystream topic 8212 (VERIFIED): Salk says the default party AI overrides player commands once you switch characters. He says the Enemy Enhancer project never finished and that behaviour changes require restructuring the AI. https://deadlystream.com/topic/8212-modfix-to-adjust-the-default-attack-mode-for-pc-and-companions/
- Deadlystream topic 7604 (VERIFIED): ranged player characters "rush the enemy as if they're holding a melee weapon". Salk points to Enemy Enhancer and Improved AI, and no mod fixes it. https://deadlystream.com/topic/7604-yet-another-post-regarding-combat-ai-or-make-companions-stop-being-useless/
- Searches on kiting, keep-distance and ranged-stance-melee-avoidance found no released mod (search result only, UNVERIFIED).

## 2. How TSL AI works

### 2.1 Event flow (VERIFIED from the vanilla TSL k_ai_master source)
Source: https://raw.githubusercontent.com/KOTORCommunityPatches/Vanilla_KOTOR_Script_Source/master/TSL/Vanilla/Data/Scripts/k_ai_master.nss

All default creature and henchman event scripts run through `k_ai_master`, switching on `GetRunScriptVar()`:

```
//   This is the AI file for the entire game.  All the default hen and def scripts run through this script.
#include "k_inc_generic" ...
int nEvent = GetRunScriptVar();
case 3001..3003: GN_DetermineCombatRound(...)           // hooks for scripted calls
// default/hostile AI: 1001 HEARTBEAT, 1002 PERCEPTION, 1003 COMBAT_ROUND_END, 1004 DIALOGUE, 1005 ATTACKED, 1006 DAMAGE, 1007 DEATH, 1008 DISTURBED, 1009 BLOCKED, 1010 FORCE_AFFECTED, 1011 GLOBAL_DIALOGUE_END, 1012 PATH_BLOCKED
// henchmen (party):     2001 HEARTBEAT, 2002 PERCEPTION, 2003 COMBAT_ROUND_END, 2004 DIALOGUE, 2005 ATTACKED, 2006 DAMAGE, 2007 DEATH, 2008 DISTURBED, 2009 BLOCKED, ..., 2013 ENTER_5m, 2014 EXIT_5m
```

Party end-of-round handler (case 2003):
```
if(!GN_GetSpawnInCondition(SW_FLAG_AI_OFF)) {
    SpeakString("GEN_I_WAS_ATTACKED", TALKVOLUME_SILENT_TALK);
    GN_DetermineCombatRound();
}
```

Party perception (2002) only engages if `GetAttackTarget(GetPartyMemberByIndex(0))` is valid.

Party heartbeat (2001) issues follow-leader when the party member is idle. It checks `GetCurrentAction != ACTION_MOVETOPOINT && != ACTION_FOLLOWLEADER`, `!GetIsConversationActive()`, `GetCommandable()`, and `!GetSoloMode()`.

Because 2003 fires every combat round, `GN_DetermineCombatRound` is re-run each round. A kiting decision made at that point is re-evaluated each round, and `ClearAllActions()` there cancels prior movement.

Other confirmations:
- https://forums.mixnmojo.com and https://deadlystream.com/topic/7437-help-making-my-first-mod-turning-off-henchman-ai/ (VERIFIED): the same event ids are the ones to disable.
- https://deadlystream.com/topic/4604-kotor-ai-scripts-for-npc/ (VERIFIED): `aiscripts.2da` only wires 3 functional scripts in the K1 era (Default 0, Grenade 4, Jedi 5). Aid, Melee, Ranged and Passive were referenced in the TLK but not implemented in K1. `GN_RespondToShout` in `k_inc_generic` checks `GetNPCAIStyle()` and `PARTY_AISTYLE_PASSIVE`.

### 2.2 AI style constants (VERIFIED, PyKotor scriptdefs)
Source: https://raw.githubusercontent.com/NickHugi/PyKotor/master/Libraries/PyKotor/src/pykotor/common/scriptdefs.py (the KOTOR_CONSTANTS and TSL_CONSTANTS lists).

```
PARTY_AISTYLE_AGGRESSIVE 0, _DEFENSIVE 1, _PASSIVE 2                    (K1 and TSL)
NPC_AISTYLE_DEFAULT_ATTACK 0, RANGED_ATTACK 1, MELEE_ATTACK 2, AID 3, GRENADE_THROWER 4, JEDI_SUPPORT 5   (K1 and TSL)
TSL adds:
NPC_AISTYLE_HEALER 6, SKIRMISH 7, TURTLE 8,
PARTY_AGGRO 9, PARTY_DEFENSE 10, PARTY_RANGED 11, PARTY_STATIONARY 12, PARTY_SUPPORT 13, PARTY_REMOTE 14,
MONSTER_POWERS 15
```

Player-facing stance names (from Steam and GameFAQs threads): Aggressive, Defensive, Ranged, Stationary, Support (UNVERIFIED as to the exact mapping). The mapping to the constants is visible in the LOCAL source below.
- https://steamcommunity.com/app/208580/discussions/0/3808403890001028954/ (VERIFIED, forum): "Giving them only ranged weapons and setting their behavior to Ranged helps a little", and "as soon as I move my cursor off it just automatically resets to Aggressive".
- https://www.gog.com/forum/star_wars_knights_of_the_old_republic_series/kotor2_combat_wiping_action_queue (VERIFIED, forum): the workaround is to switch stance to defensive, stationary or ranged instead of aggressive.

Script accessors (signatures VERIFIED in PyKotor scriptdefs):
```
// 704 int GetPartyAIStyle();
// 705 int GetNPCAIStyle(object oCreature);
// 706 void SetPartyAIStyle(int nStyle);
// 707 void SetNPCAIStyle(object oCreature, int nStyle);
```
Function ids also cross-checked in xoreos: https://raw.githubusercontent.com/xoreos/xoreos/master/src/engines/kotor2/script/function_tables.h

### 2.3 Vanilla K1 GN_DetermineCombatRound (VERIFIED web)
Source: https://raw.githubusercontent.com/KOTORCommunityPatches/Vanilla_KOTOR_Script_Source/master/K1/Data/scripts.bif/k_inc_generic.nss

```
int nPartyAI = GetPartyAIStyle();
int nNPC_AI  = GetNPCAIStyle(OBJECT_SELF);
if(!SW_FLAG_COMMONER_BEHAVIOR && !SPECTATOR_STATE && !AI_OFF && !GetUserActionsPending()) {
  if(GetPartyMemberByIndex(0) != OBJECT_SELF && !GetPlayerRestrictMode()) {
     ... boss / shields / resistances ...
     if(nNPC_AI == NPC_AISTYLE_DEFAULT_ATTACK) GN_RunDefaultAIRoutine(oIntruder);
```
`GN_RunDefaultAIRoutine` in K1 ends with `ClearAllActions(); GN_EquipAppropriateWeapon(); ActionUseTalentOnObject(...)` or `ActionAttack(oTarget)`. There is no range management at all. Engine `ActionAttack` decides the approach distance.

`GN_GetWeaponType` (K1 `k_inc_gensupport`) classifies via a BASE_ITEM_* list: 2 = ranged, 1 = melee. Engine-side `GetWeaponRanged(object)` (511) also exists in TSL.

Target selection when nothing is targeted is in `GN_DetermineAttackTarget`, which uses `GetNearestCreature(... PERCEPTION_SEEN)` plus PARTY_AISTYLE (K1 source, VERIFIED).

### 2.4 LOCAL: vanilla TSL k_inc_generic (not web; user's extracted vanilla)
File: `/home/ethansm/Games/KOTOR2_Mod_Workspace/script_recompile_20260924/tools/vanilla/k_inc_generic.nss`. Line numbers refer to that file. Also used for the `HasLineOfSight` grep.

TSL's `GN_DetermineCombatRound` dispatch (lines ~353-470) explicitly handles the TSL styles:
```
NPC_AISTYLE_SKIRMISH  -> GN_RunSkirmishAIRoutine (random move via GetRandomDestination(OBJECT_SELF,6) then ActionMoveToLocation) then Default
NPC_AISTYLE_PARTY_AGGRO   -> GN_RunDefaultAIRoutine   ("Aggressive just acts like it did in KotOR1")
NPC_AISTYLE_PARTY_DEFENSE -> if(GN_CheckRangeFromLeader(5.0f)) return; GN_RunDefaultAIRoutine
NPC_AISTYLE_PARTY_RANGED  -> /* "Ranged Support will stand back and use ranged weapons. If they are attacked by
                               someone with melee they will switch to their alternate weapon config ...
                               Must stay within 10 meters of party leader" */
                            if(GN_CheckRangeFromLeader(10.0f)) return; GN_RunRangedSupportAIRoutine(oIntruder);
NPC_AISTYLE_PARTY_STATIONARY -> GN_RunRangedSupportAIRoutine(oIntruder)   (GN_RunStationaryAIRoutine commented out)
NPC_AISTYLE_PARTY_SUPPORT -> GN_IsJedi ? GN_RunJediSupportAIRoutine : GN_RunGrenadeAIRoutine
NPC_AISTYLE_PARTY_REMOTE  -> GN_CheckRangeFromOwner(2.0f); GN_RunDefaultPuppetAIRoutine
```

`GN_RunRangedSupportAIRoutine` (line ~3706) does only this:
```
if (nCurAI == NPC_AISTYLE_PARTY_RANGED) { if (GN_CheckRangedPreferred()) return 0; }   // weapon-config swap
GN_RunDefaultAIRoutine(oIntruder);                                                     // then ordinary ActionAttack
```
`GN_CheckRangedPreferred` tracks "hit by melee" rounds in `SW_NUMBER_HEALERAI_THRESHOLD` and swaps weapon set. So TSL's "Ranged" stance has no movement-away behaviour. This is the gap the mod fills.

`GN_CheckRangeFromLeader(float)`: if `GetDistanceToObject2D(leader) > limit` and the current action is not `ACTION_MOVETOPOINT`, it runs `ClearAllActions(); ActionMoveToObject(oLeader, TRUE, fDistanceLimit/2.0f);`.

Party-defense target selection (line ~642) is weapon-aware: with a ranged weapon (`GN_GetWeaponTypeFromBaseItem == 2`) it only targets `GetAttackTarget(leader)`. With melee it only targets enemies within 5 m of the leader.

OEI's own LOS repositioning (lines ~730-830), restricted to non-party droids, with an author note documenting a known problem:
```
//RWT-OEI 04/07/04 - I added this support to make it so that ranged-only users will try to chase down the characters.
//  But the problem is that the HasLineOfSight function is returning vectors that don't actually give line of
//  sight -sometimes-. It works about 60%, but the percentage goes down the further the player is from the attacker.
} else if ( GetRacialType(OBJECT_SELF) == RACIAL_TYPE_DROID && !IsObjectPartyMember(OBJECT_SELF)) {
   vector vNewPos = GetRandomDestination( OBJECT_SELF, 10 );
   while ( nCount < 20 ) {
       if ( HasLineOfSight( vNewPos, vTargetPos ) ) break;
       vNewPos = GetRandomDestination( OBJECT_SELF, 20 ); ++nCount;
   }
   if ( nCount < 20 ) { if ( GetCurrentAction() != ACTION_MOVETOPOINT ) {
        ActionMoveToLocation( Location(vNewPos, GetFacing(OBJECT_SELF)), TRUE ); SetLocalNumber(OBJECT_SELF, 9, +1); } }
```
The mod's lateral-LOS logic can reuse this pattern, but the OEI note says `HasLineOfSight` is unreliable, so plan to verify its result independently. The TSL `nwscript.nss` documents the function as: "used in the AI to help the creatures using ranged weapons find better places to shoot when the player moves out of sight".

Other vanilla behaviour: `ActionMoveAwayFromObject(oEnemy, TRUE, 20.0)` is used in the flee routine (line ~3309).

## 3. NWScript functions in TSL

Signatures VERIFIED from PyKotor's TSL_FUNCTIONS (the doc-comments are the original nwscript.nss text). Source: https://raw.githubusercontent.com/NickHugi/PyKotor/master/Libraries/PyKotor/src/pykotor/common/scriptdefs.py. Cross-checked function ids against https://raw.githubusercontent.com/xoreos/xoreos-tools/master/src/nwscript/game_kotor2.h.

```
// 23   void  ActionMoveAwayFromObject(object oFleeFrom, int bRun=FALSE, float fMoveAwayRange=40.0f);
//        "Cause the action subject to move to a certain distance away from oFleeFrom" (fMoveAwayRange = distance to put between them)
// 360  void  ActionMoveAwayFromLocation(location lMoveAwayFrom, int bRun=FALSE, float fMoveAwayRange=40.0f);
// 820  int   HasLineOfSight(vector vSource, vector vTarget, object oSource=OBJECT_INVALID, object oTarget=OBJECT_INVALID);   // TSL only
// 511  int   GetWeaponRanged(object oItem);        // TRUE if oItem is a ranged weapon
// 316  object GetAttackTarget(object oCreature=OBJECT_SELF);      // "only works when oCreature is in combat"
// 361  object GetAttemptedAttackTarget();          // set every attack, reset at end of combat
// 721  object GetLastHostileTarget(object oAttacker=OBJECT_SELF); // "GetLastAttackTarget"
// 37   void  ActionAttack(object oAttackee, int bPassive=FALSE);
// 9    void  ClearAllActions();                    // creatures only
// 289  int   GetObjectSeen(object oTarget, object oSource=OBJECT_SELF);
// 151  float GetDistanceBetween(object oObjectA, object oObjectB);        // 0.0 if either invalid
// 319  float GetDistanceBetween2D(object oObjectA, object oObjectB);
// 298  float GetDistanceBetweenLocations(location lA, location lB);
// 21   void  ActionMoveToLocation(location lDestination, int bRun=FALSE);  // does nothing if the location is invalid or no path
// 22   void  ActionMoveToObject(object oMoveTo, int bRun=FALSE, float fRange=1.0f);
// 382  void  ActionForceMoveToLocation(location lDestination, int bRun=FALSE, float fTimeout=30.0f);
// 320  int   GetIsInCombat(object oCreature=OBJECT_SELF, int bOnlyCountReal=FALSE);
// 514  int   GetUserActionsPending();              // "test the combat action queue to see if the user has placed any actions"
// 872  int   GetCombatActionsPending(object oCreature);
// 400  void  ActionEquipMostDamagingRanged(object oVersus=OBJECT_INVALID);
// 522  int   GetCurrentAction(object oObject=OBJECT_SELF);                 // ACTION_MOVETOPOINT 0, ACTION_ATTACKOBJECT 3, ACTION_FOLLOWLEADER 38
// 27   vector GetPosition(object oTarget);  28 float GetFacing(object oTarget);  215 location Location(vector, float);  213 location GetLocation(object)
// 462  int   GetSoloMode();   701 int GetIsConversationActive();   162 void SetCommandable(int bCommandable, object oTarget=OBJECT_SELF);
```

Notes:
- The names `GetIsWeaponRanged` and `GetLastAttackTarget` do not exist as functions; use `GetWeaponRanged` and `GetLastHostileTarget`.
- No walkability or "is location valid" function exists in TSL nwscript (searched PyKotor scriptdefs, both lists). Your own probes must rely on `ActionMoveToLocation` failing silently, or on `HasLineOfSight` and `GetRandomDestination` as OEI did.
- `GetRandomDestination(object, float)` is used by OEI (LOCAL source), but I did not extract its signature from the web sources. Check the local nwscript.
- KotOR.js function table also lists these functions (ids 23, 41, 151, 289, 298, 37, 9): https://raw.githubusercontent.com/KobaltBlu/KotOR.js/master/src/nwscript/NWScriptDefK2.ts. That table is a K2 def with limited coverage, and I did not find 316 or 820 there in the fetched summary.

## 4. Engine level

### 4.1 Attack range data
- `baseitems.2da` columns (Deadlystream 2DA tutorial, https://deadlystream.com/topic/4725-detailed-information-of-2da-files/, VERIFIED but marked "probably" and "unknown" by the author):
  - `rangedweapon`: 1 for ranged weapons including grenades.
  - `maxattackrange`: "Probably the max. attack distance for ranged weapons (including grenades)".
  - `prefattackdist`: "Unknown, set to 20 for all blasters and 0.5 for everything else".
  - `minrange`, `maxrange`: undocumented.
- reone reads exactly `maxattackrange`: `_attackRange = baseItems->getInt(_baseItem, "maxattackrange");` (VERIFIED, https://raw.githubusercontent.com/seedhartha/reone/master/src/libs/game/object/item.cpp). It defaults to 2.0 m (`kDefaultAttackRange = 2.0f`) unless the right-hand weapon's range is greater (VERIFIED, `Creature::getAttackRange`, creature.cpp and creature.h).
- reone `AttackObjectAction::execute` (VERIFIED, https://raw.githubusercontent.com/seedhartha/reone/master/src/libs/game/action/attackobject.cpp):
```
// Make the actor follow its target. When reached, register an attack.
if (creatureActor->navigateTo(_attackee->position(), true, creatureActor->getAttackRange(), dt))
    _game.combat().addAttack(std::move(creatureActor), _attackee, self);
```
  So ActionAttack is "navigate until within weapon range, then attack". There is no line-of-sight check and no retreat in reone. This is an engine reimplementation and is not proof of vanilla behaviour.
- reone `MoveAwayFromObject::execute` is a stub (`// TODO: implement; complete();`), so it gives no information on vanilla semantics (VERIFIED, https://raw.githubusercontent.com/seedhartha/reone/master/src/libs/game/action/moveawayfromobject.cpp).
- KotOR.js `ModuleCreature.actionInRange` hardcodes an attack distance (VERIFIED, https://raw.githubusercontent.com/KobaltBlu/KotOR.js/master/src/module/ModuleCreature.ts):
```
return distance < ( (weaponType == 1 || weaponType == 3) ? 2.0 : 15.0 );
```
  This is a KotOR.js heuristic (melee 2.0, ranged 15.0), not the vanilla numbers. KotOR.js `ActionCombat` only schedules combat-round actions (attack, spell, equip) from `combatRound.scheduledActionList` (VERIFIED, https://raw.githubusercontent.com/KobaltBlu/KotOR.js/master/src/actions/ActionCombat.ts).
- Exact vanilla `maxattackrange` values per blaster item were not obtained from the web. Read them from the installed `baseitems.2da`.

### 4.2 Reverse-engineering resources
- Kotor-Patch-Manager (https://github.com/LaneDibello/Kotor-Patch-Manager, tree listing VERIFIED via the GitHub API): the patch list has no ranged-AI or combat-AI patch. The only combat-adjacent patches are:
  - `Post-Combat Movement Fix` (id `post-combat-movement-fix-v1`, "Fixes high-FPS movement locks after combat actions, Force powers, and item use", supports K1 GoG 1.03 and K2 GoG and Linux Steam). Manifest read.
  - `K2FinesseMeleeFix` (`GetWeaponFinesse` fallthrough). Manifest read.
  - `ScriptExtender` (adds NWScript engine functions).
- Lane's K1 GoG Ghidra database (99.9% of functions labeled): https://deadlystream.com/topic/11948-kotor-1-gog-reverse-engineering/ (VERIFIED). The thread does not document combat or attack range. It notes that virtual calls and vtables are under-labeled.
- Andastra (https://github.com/KotORPublicDomain/Andastra) is a .NET engine reimplementation. The README does not detail attack-range RE (VERIFIED).
- xoreos (https://github.com/xoreos/xoreos) has only the function tables for KotOR2 script (VERIFIED); it has no combat implementation.
- No public write-up of `swkotor2.exe` combat-round or attack-range code was found. The question "how is the approach-to-attack distance computed and does it check LOS" is open. You will need your own Ghidra work (the project already has a ghidra_knowledge DB).

## 5. Complaint confirmation

- https://deadlystream.com/topic/7604-yet-another-post-regarding-combat-ai-or-make-companions-stop-being-useless/ (VERIFIED): ranged PCs "rush the enemy as if they're holding a melee weapon". Combat starts with the weakest attack. No fix exists.
- https://steamcommunity.com/app/208580/discussions/0/3808403890001028954/ (VERIFIED): companions rushing into combat. Workarounds are ranged-only gear, the Ranged stance ("helps a little"), manual pausing, Solo Mode, and the Community Patch. Bug: stance selector reverting to Aggressive unless you click to confirm.
- https://steamcommunity.com/app/208580/discussions/0/528398719806983540/ (VERIFIED): "In Aggressive stance they will attack everything within range". The workaround is to set stances.
- https://www.gog.com/forum/star_wars_knights_of_the_old_republic_series/kotor2_combat_wiping_action_queue (VERIFIED): party AI overrides queued player actions, and stance change is the workaround.
- GameFAQs "Behavior" thread (https://gamefaqs.gamespot.com/boards/920601-star-wars-knights-of-the-old-republic-ii-the-sith-lords/60923455) returned 403. The stance descriptions (Ranged "use ranged until enemies close to melee range", Defensive "within 5 m") come from a search-result summary (UNVERIFIED). The 5 m Defensive value matches the LOCAL source (`GN_CheckRangeFromLeader(5.0f)`), while the LOCAL comment text says 10 m in one place and the code uses 5.0.

## 6. Implications for the mod design

- Best hook: the party `k_ai_master` event 2003 (COMBAT_ROUND_END) plus 2005 and 2006 (attacked, damaged), so the routine re-evaluates each round. Alternatively, extend `GN_RunRangedSupportAIRoutine`, which is the exact place where TSL's Ranged stance runs. Adding kite and LOS logic before `GN_RunDefaultAIRoutine` there is the least invasive change.
- Use `GetWeaponRanged(GetItemInSlot(INVENTORY_SLOT_RIGHTWEAPON))` or `GN_GetWeaponTypeFromBaseItem` for "is ranged".
- Kite: `ClearAllActions(); ActionMoveAwayFromObject(oMelee, TRUE, fRange);`. Guard with `GetCurrentAction() != ACTION_MOVETOPOINT` and keep the leader leash (`GN_CheckRangeFromLeader`) so members do not wander off. Avoid `GetUserActionsPending()` conflicts so player commands still win.
- LOS: copy OEI's pattern (`GetRandomDestination` plus `HasLineOfSight` retry loop, 20 tries). Add a post-check with `GetObjectSeen(oTarget)` on arrival, because OEI documented about 60% reliability.
- Conflict risk: every AI mod replaces `k_ai_master.ncs`. Merge with Improved AI (TSLRCM-compatible, has optional source) rather than overriding it.
- Unknowns to resolve with Ghidra or in-game logging: what the engine uses for ActionAttack approach distance (`maxattackrange` vs `prefattackdist`), and whether the engine re-issues move-to-attack after our `ActionMoveAway` completes.
