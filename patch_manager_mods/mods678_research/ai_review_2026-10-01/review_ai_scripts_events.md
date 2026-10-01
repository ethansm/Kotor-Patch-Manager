# KOTOR II companion AI: review of the event scripts (getting into and out of combat, out-of-combat behaviour)

Reviewer scope: `k_hen_*` event scripts, the `k_ai_master` event cases, `k_hen_<name>dlg`, `k_oei_*` (the companions' real spawn, end-conversation and user-defined scripts), `a_atkonend`, `a_force_combat`, `k_combat_rnd`, and the Mod 8 kite routine. The combat decision layer (`GN_DetermineCombatRound` target, talent and weapon choice) belongs to the other reviewer. I touch it only where it decides whether a companion leaves combat.

Line numbers are 1-based with CR stripped. `k_ai_master.nss` and `k_inc_generic.nss` in `vanilla/` and `vanilla_all/` are byte-identical (checked with `cmp`). The live `override/k_ai_master.ncs` has md5 `94db8304…`, which equals `mod8_kite_script/out/k_ai_master.ncs` (the Mod 8 build).

**New fact the brief did not have.** I parsed the live `override/p_*.utc` files and the BIF templates with pykotor (read-only). The K2 companions do **not** use `k_hen_dialogue01` or `k_hen_spawn01`:

| UTC event | Atton, Bao-Dur, Disciple, G0T0, Handmaiden, Hanharr, HK-47, Kreia, Mand, Mira, T3-M4, Visas, Remote |
|---|---|
| OnDialogue | `k_hen_<name>dlg` (its own compiled copy of `k_inc_generic`, so it does **not** go through `k_ai_master`) |
| OnSpawn | `k_oei_spawn` (Visas: `m_spn_visas` from mod142 Improved Force Sight, which `ExecuteScript`s `k_oei_spawn` and then applies Force Sight once, guarded by local boolean 118) |
| OnEndDialogue | `k_oei_endconv`, which runs `k_ai_master` case **1011** (a default event, not a hench event) |
| OnUserDefined | `k_oei_userdef`. The live override copy comes from mod148 Obscured Tomb (md5 `d81914b2`) and only changes the 711KOR handling. |
| OnSpellCastAt | empty for every companion (so case 1010/2010 never fires for them) |
| heartbeat / notice / attacked / damaged / end-round / blocked | `k_hen_*01` → `k_ai_master` 2001 / 2002 / 2005 / 2006 / 2003 / 2009 |

`swkotor2.exe` hard-codes `k_hen_leadchng`, `k_hen_enter5m`, `k_hen_exit5m`, `k_hen_retreat` and `k_def_pathfail01` (from `strings`). `k_def_pathfail01` runs `k_ai_master` 1012 on every non-leader creature, party members included. `k_combat_rnd` is **not** referenced by the exe, despite its comment.

---

## Summary

| id | severity | confidence | symptom (one line) |
|---|---|---|---|
| E1 | **Medium** | verified (code path); symptom plausible | Ranged, Stationary, Support and Kite companions never run the combat-exit routine (`GN_DoPostDCRChecks`: CancelCombat + follow). They stand idle after the last kill until the next heartbeat, up to 6 s or longer. |
| E2 | **Medium (Mod 8)** | verified | Every shout-driven entry into combat for named companions goes through `k_hen_<name>dlg` with the *vanilla* include. Stance 16 then runs the plain default routine: no 10 m leash, no kite, no LOS reposition. |
| E3 | Medium-Low | verified (code); symptom plausible | The shout handler makes companions **run to 2 m** of an unseen enemy that hurt the leader, whatever weapon they hold. Nothing is queued after the move. It bypasses the leash and the E-H3 engine rule. |
| E4 | Medium-Low | verified (code); symptom plausible | Any `MOVETOPOINT` freezes a Ranged/Kite/Defensive companion's AI while it is more than the leash distance away. The heartbeat also skips `MOVETOPOINT`, so script never recovers from a stuck move (this amplifies the engine post-combat movement lock). |
| E5 | Low-Medium | verified (code); symptom plausible | Heartbeat and leader change wipe player-issued non-combat orders (open lock, disarm mine, use object, pick up) on a companion you just switched away from. |
| E6 | Low-Medium | verified | "Hit by melee" mark (local number 8) is set for unarmed or creature-weapon attackers, never cleared at combat end, and collides with the Remote's droid-heal flag. The result is a stale weapon config in the next fight and Remote healing turned on by accident. |
| E7 | Low | verified | Exit-5m and enter-5m hooks are no-ops (their case bodies are commented out), but the heartbeat comment says distance-follow moved there. Follow only resumes on the 6 s heartbeat; "join when the leader attacks" only works through shouts. |
| E8 | Low | plausible | A dead last hostile target can still be "attacked" (`GetLastHostileTarget` has no not-dead guarantee). `GN_RunDefaultAIRoutine` then returns TRUE, so even Aggressive companions skip the combat exit. Overlaps the decision layer. |
| E9 | Low | verified | The shout-15 (`GEN_COMBAT_ACTIVE`) branch in `GN_RespondToShout` is unreachable dead code. Companions never answer allied NPCs' combat shouts. |
| E10 | Low | verified (code); symptom plausible | The blocked handler bashes any bashable locked door in the way (party AI elsewhere refuses to bash). It has no validity check. Its INT gates are moot (every companion has INT ≥ 10). |
| E11 | Low | verified | The damage handler runs a companion to the **exact location** of an unseen damager with no stand-off. Its "not fighting" check runs right after queueing the DCR action, so the move is appended behind the new attack. |
| E12 | Low (Mod 8) | verified | Kite cooldown (local number 20) is not reset at combat end, so the next fight's first DCR can skip the kite check. No index-20 collision exists on party members. |
| E13 | Low | verified (code); effect plausible | The leader's OnAttacked shouts three times. Each idle companion can run `ClearAllActions`+DCR up to 3× per hit, which burns the per-call counters (melee mark, kite cooldown). |
| side | n/a (enemies) | verified | Combat-zone check typo in `GN_RespondToShout` (`SW_NUMBER_LAST_COMBO` instead of `SW_NUMBER_COMBAT_ZONE`). Zoned NPCs ignore same-zone shouts. No companion impact. |

---

## E1: Four of the five menu stances skip the combat-exit routine (Medium; code verified, symptom plausible)

**Where.** `vanilla/k_inc_generic.nss` (also in the live `src/k_inc_generic.nss`):

```c
402|                    GN_RunDefaultAIRoutine(oIntruder);      // PARTY_DEFENSE (10)
403|                    return;
419|                    GN_RunRangedSupportAIRoutine(oIntruder); // PARTY_RANGED (11)
420|                    return;
431|                    GN_RunRangedSupportAIRoutine(oIntruder); // PARTY_STATIONARY (12)
432|                    return;
442|                        GN_RunJediSupportAIRoutine(oIntruder); // PARTY_SUPPORT (13)
447|                        GN_RunGrenadeAIRoutine(oIntruder);
449|                    return;
```

Live Mod 8 branch (`src/k_inc_generic.nss:433-434`): `GN_RunKiteAIRoutine(oIntruder); return;`

The only party-member combat exit is reached after the dispatch:

```c
484|    if(GN_DoPostDCRChecks())
...
533|    else if(GetPartyMemberByIndex(0) != OBJECT_SELF &&
534|            !GetIsObjectValid(GetNearestCreature(CREATURE_TYPE_REPUTATION, REPUTATION_TYPE_ENEMY, OBJECT_SELF,1, CREATURE_TYPE_PERCEPTION, PERCEPTION_SEEN)) &&
535|            IsObjectPartyMember(OBJECT_SELF))
...
566|                CancelCombat(OBJECT_SELF);
568|                ClearAllActions();
569|                ActionFollowLeader();
```

**Why it is wrong.** The menu stances (`aiscripts.2da`) are Aggressive 9, Ranged 11, Stationary 12 and Jedi Support 13 (Defensive 10 is hidden), plus Mod 8's stance 16. Only Aggressive can fall through to line 484, and only when `GN_RunDefaultAIRoutine` returns FALSE.

For the other four, case 2003 (`k_ai_master.nss:634`) runs DCR after the last kill, and that DCR never calls `CancelCombat` or `ActionFollowLeader`. Also note `GN_RunRangedSupportAIRoutine` always returns 0, so the routine's result could not be used as a "did nothing" signal even if the caller checked it.

The only script path back to following is then the heartbeat (`k_ai_master.nss:528-554`, every 6 s). It needs all of these:
- the current action is not `MOVETOPOINT` (see E4);
- `!GN_GetIsFighting` (no attempted attack, spell or attack target);
- no seen enemy.

Exit-5m does nothing (E7).

**Player-visible symptom.** After a fight, Ranged, Stationary, Support and Kite companions stand still while the leader walks away. They catch up 0-6 s later, or later still if a leftover attack target or move action blocks the heartbeat conditions. Aggressive companions start following at the end of the round. Combat state also lingers (no `CancelCombat`).

**Confirm.**
- In game: put one companion on Aggressive and one on Ranged. Kill the last enemy and walk off at once. Compare when each starts following.
- In a log: make a temporary `k_ai_master` build with `SendMessageToPC(GetFirstPC(), GetTag(OBJECT_SELF)+" 2003 act="+IntToString(GetCurrentAction())+" inCombat="+IntToString(GetIsInCombat()))` in cases 2003 and 2001. The feedback log shows which event restarted following and when.
- The ranged-ai DLL log (`ranged_ai_log.txt`) shows whether an attack action survives the kill.

**Fix options** (only `k_inc_generic` changes; recompile with nwnnsscomp via `build_kite_script.py`):
- At the top of the party branch of `GN_DetermineCombatRound`: if there is no seen enemy and `GetPartyMemberByIndex(0) != OBJECT_SELF`, run `GN_DoPostDCRChecks(); return;`.
- Or, after each `GN_Run…Routine` + `return` in branches 10/11/12/13/16, call `GN_DoPostDCRChecks()` when no enemy is seen.

Recompile `k_ai_master` (the live 2003 path). For consistency also recompile the 13 `k_hen_<name>dlg`, `a_force_combat` and `k_combat_rnd` (E2), because each embeds its own copy of DCR.

---

## E2: Shout-driven DCR for named companions runs the vanilla include, so stance 16 gets the default routine (Medium for Mod 8; verified)

**Where.** All 13 `vanilla_all/k_hen_*dlg.nss` have the same `main()` (md5 of the extracted bodies `7ee9bcfc…`):

```c
#include "k_inc_generic"
...
            else
            {
                GN_RespondToShout(oShouter, nMatch, GetLastHostileActor(oShouter));
            }
```

`GN_RespondToShout` calls `GN_DetermineCombatRound(oIntruder)` (`k_inc_generic.nss:2033, 2073, 2081`). That DCR is compiled into each `k_hen_*dlg.ncs`. None of these is in `override/` (checked), so the BIF vanilla build runs. In it, stance 16 falls through to:

```c
473|                else
474|                {
475|                    //Run a default AI no matter what. RWT-OEI 08/20/04
476|                    if (GN_RunDefaultAIRoutine(oIntruder) == TRUE)
```

**Why it matters.** The main way an idle companion enters a fight is a shout:
- the leader's (or another companion's) round-end shout, `k_ai_master.nss:633` `SpeakString("GEN_I_WAS_ATTACKED", TALKVOLUME_SILENT_TALK)`;
- the OnAttacked shouts at lines 679, 705 and 726.

Perception (2002) only engages if the leader already has an attack target. On that first DCR a stance-16 companion gets no `GN_CheckRangeFromLeader(10)` leash, no kite step, no LOS reposition, and no reset of the melee mark. From the next end-of-round onward, case 2003 runs the live `k_ai_master` and kiting starts.

The engine hook E-H3 still stops the attack approach, so the visible effect is limited to the first DCR. The same applies to `a_force_combat` and `k_combat_rnd` if a dialog or module ever runs them on a companion (not visible from source).

**Symptom.** On the opening round, a Kiting companion that starts next to a melee enemy shoots from where it stands instead of stepping away. It can also be beyond 10 m of the leader for the first round.

**Confirm.** Add `SendMessageToPC` at the top of `GN_RunKiteAIRoutine`. In a fight started by the PC, the first log line for that companion should come one round after it starts attacking.

**Fix options.**
- (a) Recompile the 13 `k_hen_*dlg.nss` (sources in `vanilla_all/`) against the Mod 8 `src/k_inc_generic.nss` and install them to `override/`.
- (b) Better: change their `else` branch to `ExecuteScript("k_ai_master", OBJECT_SELF, KOTOR_HENCH_EVENT_ON_DIALOGUE);`. Case 2004 does exactly the same `GN_RespondToShout`. Future include changes then only need `k_ai_master`. Keep `SelectAndRunDLG()` for `nMatch == -1`.

Optionally do the same for `a_force_combat` and `k_combat_rnd`: use `ExecuteScript("k_ai_master", OBJECT_SELF, 3001)`.

---

## E3: Shout response runs companions to 2 m of an unseen intruder (Medium-Low; code verified, symptom plausible)

**Where.** `k_inc_generic.nss:2027-2063` (in `GN_RespondToShout`, shout 1, party listener):

```c
2027|                                if(GetObjectSeen(oIntruder))
...
2035|                                else if(GetIsObjectValid(oIntruder))
2036|                                {
2039|                                    ClearAllActions();
2049|                                    if(GetDistanceBetween(OBJECT_SELF, oIntruder) < 20.0 && !GetObjectSeen(oIntruder))
2050|                                    {
2051|                                        ActionMoveToObject(oIntruder, TRUE, 2.0);
2052|                                    }
2053|                                    else
2054|                                    {
2055|                                        if(GN_GetWeaponType(OBJECT_SELF) == 1)
2057|                                            ActionMoveToObject(oIntruder, TRUE, 4.0);
2061|                                            ActionMoveToObject(oIntruder, TRUE, 15.0);
```

`oIntruder` is `GetLastHostileActor(oShouter)` (dlg scripts, and `k_ai_master.nss:660`). That is the last enemy that hurt the *leader*, not the leader's current target.

**Why it is wrong.** Inside the `else if` the companion cannot see the intruder, so `!GetObjectSeen(oIntruder)` is always TRUE. Every unseen intruder within 20 m therefore gets a 2 m run, whatever the weapon. The weapon-based stand-off (4 m melee, 15 m ranged) only applies to enemies *farther* than 20 m. That is the reverse of the "reduce NPCs running up to enemies with blasters" comment.

Nothing is queued after the move: no `ActionAttack`, no `ActionDoCommand(DCR)`. The move also bypasses `GN_CheckRangeFromLeader` and the engine E-H3 rule, because it is a script move, not an attack approach.

**Symptom.** When the leader is shot by something a Ranged or Kite companion can't see (a stealthed enemy, around a corner, in a dark spot or beyond perception range), the companion sprints right up to it with a blaster. It then stands there until another event (round end, attacked, perception) gives it a DCR.

**Confirm.** Let a stealthed or out-of-sight sniper hit the PC while a Ranged companion stands idle 10-15 m away. The ranged-ai DLL log shows no attack action during the run. A `SendMessageToPC` in this branch confirms the path.

**Fix options** (`k_inc_generic` → recompile `k_ai_master` and the dlg scripts per E2):
- Drop the `< 20.0` clause and use the weapon-based range always.
- For party members, append `ActionDoCommand(GN_DetermineCombatRound(oIntruder))`.
- Or prefer `GetAttackTarget(oShouter)`, then `GetSpellTarget(oShouter)`, before the move.
- Mod 8: skip the move entirely for stances 11, 12 and 16 (use `ActionFollowLeader` instead).

---

## E4: `MOVETOPOINT` freezes the leash check, and the heartbeat never recovers a stuck move (Medium-Low; code verified, symptom plausible)

**Where.** `k_inc_generic.nss:3634-3641` (`GN_CheckRangeFromLeader`, used by stances 10, 11 and 16):

```c
3634|    int nActionID = GetCurrentAction(OBJECT_SELF);
3636|    if ( nActionID == ACTION_MOVETOPOINT )
3637|    {
3638|        //AurPostString( "Actively trying to move to leader.", 10, 34, 2.0f );
3639|        //Leave alone
3640|        return 1;
3641|    }
```

`k_ai_master.nss:534`: `GetCurrentAction(OBJECT_SELF) != ACTION_MOVETOPOINT &&` (heartbeat gate).

**Why it is wrong.** The leash assumes any `MOVETOPOINT` is its own move to the leader. Other moves are also `MOVETOPOINT`:
- the shout run to an intruder (E3);
- the 2006 run to a damager (E11);
- a kite move (`kite_routine.nss:115`);
- scripted moves.

While the companion is beyond the leash distance and any of these moves is active, every DCR returns at once and the companion does nothing in combat. The heartbeat deliberately leaves `MOVETOPOINT` alone, so if a move never completes, no script path ever clears it. One way a move never completes is the engine's high-FPS movement lock, which the community "post-combat-movement-fix" (x87 compare at `0x7AF198`, ghidra DB `hooks_ported`) addresses.

Script does not cause the lock, but it gives no recovery path. This is the script-side contribution to "party members don't move or follow after combat".

**Symptom.** A companion keeps "running" (or stands frozen) for good after a fight. It does not answer DCR, follow or heartbeat until the player gives it a direct order.

**Confirm.** Log `GetCurrentAction()` and `GetAttemptedMovementTarget()` from the 2001 heartbeat via `SendMessageToPC`. A stuck companion shows `act=0` heartbeat after heartbeat.

**Fix options** (recompile `k_ai_master`, and the dlg scripts if the include changes):
- In `GN_CheckRangeFromLeader`, return 1 only when `GetAttemptedMovementTarget() == oLeader`. Otherwise `ClearAllActions(); ActionMoveToObject(oLeader, TRUE, fDistanceLimit/2.0)`.
- In 2001, count consecutive heartbeats with `MOVETOPOINT` and an unchanged position (use a free local number, for example 23). After 2-3 heartbeats, `ClearAllActions(); ActionFollowLeader();`.

---

## E5: Heartbeat and leader change wipe the player's queued non-combat orders (Low-Medium; code verified, symptom plausible)

**Where.** `k_ai_master.nss:532-553` (2001):

```c
534|                       GetCurrentAction(OBJECT_SELF) != ACTION_MOVETOPOINT &&
536|                       GetCurrentAction(OBJECT_SELF) != ACTION_FOLLOWLEADER &&
...
548|                        if(!GN_GetIsFighting(OBJECT_SELF) && !GetIsObjectValid(oEnemy))
552|                            ClearAllActions();
553|                            ActionFollowLeader();
```

`vanilla_all/k_hen_leadchng.nss:18-37` (engine-called on leader change; the `MOVETOPOINT` exclusion is commented out):

```c
20|               //GetCurrentAction(OBJECT_SELF) != ACTION_MOVETOPOINT &&
22|               GetCurrentAction(OBJECT_SELF) != ACTION_FOLLOWLEADER &&
...
36|                        ClearAllActions();
37|        		        ActionFollowLeader();
```

**Why it is wrong.** The check is a blacklist of two actions, with no `GetUserActionsPending()`. Note that `nwscript.nss:4495-4498` says `GetUserActionsPending` "will only work during combat", so it would not help out of combat anyway. As a result, every other action is cancelled: `ACTION_OPENLOCK` (13), `DISABLETRAP`/`RECOVERTRAP`/`FLAGTRAP` (8-10, mines), `USEOBJECT` (15), `PICKUPITEM` (1), `HEAL` (33), `WAIT` (36) and looping animations.

`GN_GetIsFighting` only protects attack and spell targets. The commented-out 2013 code (`k_ai_master.nss:834`, `if(!GetUserActionsPending())`) shows the designers knew about this protection.

**Symptom.** Control Atton, order him to disarm a mine or slice a footlocker, then switch back to the PC. With `k_hen_leadchng` he drops the task at once and follows. If you switch before he starts, the next heartbeat (≤6 s) cancels it.

**Confirm.** Do exactly that with a mine and a locked container. The task is never completed.

**Fix options.**
- 2001 (in `k_ai_master`): re-issue the follow only when `GetCurrentAction() == ACTION_QUEUEEMPTY`, or for a short whitelist (for example `ACTION_FOLLOW`, `ACTION_INVALID`).
- `k_hen_leadchng.nss`: own script, recompile it and put it in `override/` (the engine looks it up by name). Apply the same whitelist, or restore the `MOVETOPOINT` exclusion and add OPENLOCK, the trap actions, USEOBJECT and PICKUPITEM.

Possibly intended ("switching cancels orders"), so ask the user before changing it.

---

## E6: The "hit by melee" mark (local number 8) is misclassified, stale and shared (Low-Medium; verified)

**Where.** `k_ai_master.nss:685-699` (2005, every party member and the leader):

```c
685|            object oAttacker = GetLastAttacker();
686|            object oWeapon = GetItemInSlot(INVENTORY_SLOT_RIGHTWEAPON,oAttacker);
688|            int nWeaponType = 0;
689|            if ( GetIsObjectValid(oWeapon))
691|                nWeaponType = GN_GetWeaponTypeFromBaseItem(GetBaseItemType(oWeapon));
693|            if ( nWeaponType == 1 || nWeaponType == 0)
698|                SetLocalNumber( OBJECT_SELF, SW_NUMBER_HEALERAI_THRESHOLD, 2);
```

The only decrement is `k_inc_generic.nss:3773-3778` (`GN_CheckRangedPreferred`, stances 11 and 16, one per DCR call). The only reset is the kite's `kite_routine.nss:120`. There is no reset at combat end.

`k_oei_hench_inc.nss:1334` (Remote out-of-combat user-defined heartbeat) reads the same index:

```c
1334|            if ( GetLocalNumber(OBJECT_SELF, SW_NUMBER_HEALERAI_THRESHOLD ) )
1336|                nThreshold = 90;
```

**Why it is wrong.**
- (a) `nWeaponType == 0` counts as "melee" for: attackers with no right-hand weapon; anything outside the 29 hard-coded base items (creature-slot weapons on droids, turrets and beasts, and base items added by mods); and an invalid `GetLastAttacker()`. A droid shooting with a built-in creature weapon marks the companion as "in melee".
- (b) The mark survives the end of combat, because no DCR runs after the fight for stances 11 and 16 (E1). The next fight starts with a stale mark: the first DCR keeps or switches to the melee config (`3800-3806`), and a Ranged companion that swapped to melee late in a fight walks out holding the melee config.
- (c) Index 8 is also the Remote's "heal droids" flag. In vanilla source nothing else writes it (only `k_ai_master:698`; a dialog param script `a_localn_set` could). So a Remote hit in melee permanently gains free out-of-combat droid healing (`EffectHeal` every 2 s heartbeat).

**Symptom.**
- Ranged-stance companions swap to their melee config when shot by droids or turrets, or at the start of a fight that follows a melee fight.
- They stay in the melee config after combat.
- The Remote heals party droids after it has been bitten once.

**Confirm.** `SendMessageToPC` the value of `GetLocalNumber(OBJECT_SELF,8)` in 2001 for each companion. It stays at 1-2 between fights.

**Fix options.**
- Classify by the attacker's right hand, falling back to `INVENTORY_SLOT_CWEAPON_*` or `GetWeaponRanged()`. Mark only when type == 1 (or type 0 with distance ≤ 3 m).
- Move the mark to a dedicated free index (for example 23; vanilla does not use it).
- Clear it, together with kite local 20, in `GN_DoPostDCRChecks`'s party branch (pairs with E1).

Recompile `k_ai_master`, plus the dlg scripts if the include changes.

---

## E7: Exit-5m and enter-5m are dead hooks, so follow restarts only on the heartbeat (Low; verified)

**Where.** `k_ai_master.nss:828-884`: the bodies of cases 2013 and 2014 are entirely inside `/* … */`. `k_hen_exit5m.nss:13` and `k_hen_enter5m.nss:13` only call `ExecuteScript("k_ai_master", …, 2013/2014)`. The engine fires them by name (both strings are in `swkotor2.exe`). The heartbeat says:

```c
551|                            //The distance checking is now down in the script fired from AI Action Update - Leave 5m Radius of party leader.
```

**Why it is wrong.** The heartbeat delegates distance-based follow to exit-5m, which does nothing. Any interruption that leaves a companion idle (E1 combat end, door action, kite or leash move ending, a conversation it doesn't own) is fixed only by the next 6 s heartbeat.

Enter-5m's disabled code was the "leader is attacking near me, so join" path. Without it, idle companions next to a fighting leader join only through shouts (≤1 round) or perception. Perception needs `GetAttackTarget(oLeader)` (`k_ai_master.nss:602`), so a leader who opens with Force powers or grenades does not trigger it.

**Symptom.** Companions lag 0-6 s behind the leader after any interruption. When the leader opens with a Force power, they join a round late.

**Confirm.** Temporarily put `SendMessageToPC(GetFirstPC(),"exit5m "+GetTag(OBJECT_SELF))` in case 2014 to measure how often the engine fires it. Do this before enabling any logic.

**Fix options.** Restore the 2014 body (`k_ai_master.nss:849-881`) with a `GetCurrentAction()==ACTION_QUEUEEMPTY` guard (see E5). For 2013, restore it with its existing `GetUserActionsPending()` guard but without the unconditional `SetCommandable(TRUE)`. Recompile `k_ai_master` only. Measure the event rate first.

---

## E8: A corpse as last target keeps `GN_RunDefaultAIRoutine` "successful", so even Aggressive skips the exit (Low; plausible)

**Where.** `k_inc_generic.nss:641` `oTarget = tPR.oLastTarget;` comes from `2790` `tPR.oLastTarget = GetLastHostileTarget();`. The checks at `708-731` reject a dead target only for the "keep" case, then leave `oTarget` unchanged when nothing else is found. After that:

```c
865|    if(GetIsObjectValid(oTarget))
868|        ClearAllActions();
874|        if(GetIsTalentValid(tUse) && GetIsEnemy(oTarget))
877|            ActionUseTalentOnObject(tUse, oTarget);
878|            return TRUE;
880|        else if(GetIsEnemy(oTarget))
883|            ActionAttack(oTarget);
884|            return TRUE;
```

**Why it is wrong.** `GetLastHostileTarget` (`nwscript.nss:5355-5357`) has none of the "never a dead/destroyed creature" guarantee that `GetLastHostileActor` documents (`nwscript.nss:4713`). Until the corpse object is destroyed, the routine queues an attack or talent on it and returns TRUE. `GN_DoPostDCRChecks` is skipped even for Aggressive, so follow waits for the heartbeat, and the heartbeat waits for `GN_GetIsFighting` to clear.

**Symptom.** After the final kill, companions do a pointless "attack" or Force-power wind-up toward the body, or just stand, before following.

**Confirm.** Log `GetIsDead(GetLastHostileTarget())` and the routine's result in 2003 right after the last kill.

**Fix.** `if (GetIsDead(oTarget)) oTarget = OBJECT_INVALID;` before line 835. This is decision-layer code, so pass it to the other reviewer. Recompile all DCR carriers (k_ai_master, the 13 dlg scripts, a_force_combat, k_combat_rnd).

---

## E9: Shout 15 (`GEN_COMBAT_ACTIVE`) is unreachable (Low; verified)

**Where.** `k_inc_generic.nss:2010-2108`. The `else if(nShoutIndex == 15 …)` at 2087 is the `else` of the condition at 2022:

```c
2022|                            if((IsObjectPartyMember(OBJECT_SELF) && !GetPlayerRestrictMode()) || !IsObjectPartyMember(OBJECT_SELF))
...
2087|                            else if(nShoutIndex == 15 && GetIsFriend(oShouter) && oShouter != OBJECT_SELF)
2089|                                if(GetCurrentAction(OBJECT_SELF) == ACTION_INVALID)
```

**Why it is wrong.**
1. It sits inside `if(nShoutIndex == 1 …)` (line 2010), so `nShoutIndex == 15` can never be true there.
2. Line 2019 already checked `!GetPlayerRestrictMode()`, so the condition at 2022 is always TRUE and its `else` never runs.
3. Even if reached, an idle creature reports `ACTION_QUEUEEMPTY`, not `ACTION_INVALID`.

Pattern 15 is registered (`2170`) and shouted by every non-party creature at each round end (`k_ai_master.nss:171`). The shout-1 branch accepts party shouters only, for party listeners (`2012`).

**Symptom.** Companions never answer allied NPCs' combat shouts: Telos or Onderon soldiers, summoned allies, or mod allies such as the For Mandalore troopers. They stand by while allies fight until the leader engages or they get hit.

**Fix (optional, a behaviour change).** Move the shout-15 block to the same level as the `nShoutIndex == 1` test and use `ACTION_QUEUEEMPTY`. Recompile `k_ai_master` and the dlg scripts.

---

## E10: The blocked handler bashes locked doors (Low; code verified, symptom plausible)

**Where.** `k_ai_master.nss:807-820`:

```c
807|            object oDoor = GetBlockingDoor();
810|                if(GetAbilityScore(OBJECT_SELF, ABILITY_INTELLIGENCE) >= 5)
812|                    if(GetIsDoorActionPossible(oDoor, DOOR_ACTION_OPEN) && GetAbilityScore(OBJECT_SELF, ABILITY_INTELLIGENCE) >= 7 )
814|                        DoDoorAction(oDoor, DOOR_ACTION_OPEN);
816|                    else if(GetIsDoorActionPossible(oDoor, DOOR_ACTION_BASH))
818|                        DoDoorAction(oDoor, DOOR_ACTION_BASH);
```

**Why it is wrong.** There is no `GetIsObjectValid(oDoor)` check (case 1009 has one; probably harmless). For a locked, non-plot door, OPEN is impossible, so a following companion starts bashing it. The party's own post-DCR logic says party members should not bash (`k_inc_generic.nss:546` "Party members won't help with bashing anymore"). Bashing also starts combat state, and that door's OnAttacked/OnDamaged scripts can fire. The INT gates never matter in practice: every companion UTC has INT 10-16 (read from `override/p_*.utc`).

**Symptom.** A companion hacks at a locked door that the leader reached the far side of by another route, or that the player meant to slice.

**Confirm.** Lead the party around a locked, bashable door so that the follow path crosses it.

**Fix.** For party members, try OPEN only. On a locked door, `ClearAllActions(); ActionFollowLeader();` or do nothing. Recompile `k_ai_master`.

---

## E11: The damage handler runs to the damager's exact spot and appends the move behind the new attack (Low; verified)

**Where.** `k_ai_master.nss:750-766`:

```c
750|                    if(GetCurrentAction() != ACTION_FOLLOWLEADER)
...
758|                                    GN_DetermineCombatRound();
759|                                    if(!GN_GetIsFighting(OBJECT_SELF))
762|                                        if(!GetObjectSeen(oTarget) && GetArea(OBJECT_SELF) == GetArea(oTarget))
764|                                            ActionMoveToLocation(GetLocation(oTarget), TRUE);
765|                                            ActionDoCommand(GN_DetermineCombatRound());
```

**Why it is wrong.**
1. There is no stand-off distance. The default-event twin (1006, `321-331`) uses 2, 4 or 15 m by weapon; 2006 runs onto the damager's exact location.
2. `GN_GetIsFighting` reads the attempted or attack target, which is normally set only when a queued attack begins executing. Right after DCR has queued `ActionAttack(X)`, the test is still "not fighting", so the run is appended *behind* the new attack.
3. The `FOLLOWLEADER` gate means a following companion ignores damage altogether. That is probably intended, but note that 2005 calls DCR without an intruder (`717`), and its post-DCR (`569`) re-issues `ActionFollowLeader`. As a result, 2006's unseen-attacker logic almost never runs for a companion hit while following.

**Symptom.** Idle or solo-mode companions damaged by an unseen enemy run into its position. Occasionally, after killing a seen enemy, a companion runs off to where an earlier unseen shooter stood.

**Fix.** Use `ActionMoveToObject(oTarget, TRUE, ranged?15.0:4.0)`. Drop the move when DCR already queued an action (compare `GetCurrentAction()` before and after, or check `GetCombatActionsPending`). Recompile `k_ai_master`.

---

## E12: Kite cooldown (local number 20) leaks into the next fight (Low, Mod 8; verified)

**Where.** `kite_routine.nss:135-144`:

```c
135|    int nCool = GetLocalNumber(OBJECT_SELF, SW_NUMBER_KITE_COOLDOWN);
136|    if (nCool > 0)
138|        SetLocalNumber(OBJECT_SELF, SW_NUMBER_KITE_COOLDOWN, nCool - 1);
140|        if (GetCurrentAction(OBJECT_SELF) == ACTION_MOVETOPOINT)
142|            return 1;
```

It is set to 1 or 2 at line 117 and never cleared at combat end (E1: no exit routine for stance 16).

**Why it is wrong.** The next fight's first one or two DCRs skip the kite check. If the companion is in a `MOVETOPOINT` at that moment (a shout run per E3, or the leash move), the routine returns without acting. Locals are saved, so this also survives save and load.

**Index-20 collision check (requested).** No collision.
- Vanilla: there is no `Set/GetLocalNumber(…, 20)` in any of the 638 `.nss` files. `SW_NUMBER_*` are 3-11, `WALKWAYS_*` are 0-2, and the literals in use are 0, 8, 9, 11-16 and 29 (EXCERPTS §8 confirmed).
- Installed mods with source: mod120 For Mandalore uses 16-22 (20 = "leaving"), but only on summoned *non-party* Mandalorians (`CreateObject`, `m_act_mandsummon.nss:188`). Those can never have stance 16.
- Visas's `m_spn_visas` uses local *boolean* 118.
- Not visible: dialog parameter scripts (`a_localn_*`) and source-less override `.ncs`.

**Fix.** Clear local 20 (and local 8, see E6) in the party branch of `GN_DoPostDCRChecks`. Alternatively, treat `GetIsInCombat()==FALSE` at entry as a reset. Recompile `k_ai_master`.

---

## E13: Triple shout on the leader's OnAttacked burns the per-call counters (Low; code verified, effect plausible)

**Where.** `k_ai_master.nss:679` (`SILENT_SHOUT`), `703-706` (leader: `SILENT_TALK`), `723-727` (the `else` of "not leader": the leader shouts again).

**Why it matters.** For each hit on the leader, every idle companion receives up to three pattern-1 shouts. Each passes the "not fighting yet" gate (`k_inc_generic.nss:2016`) until the queued attack starts executing, so each runs `ClearAllActions()` + DCR. The melee mark (E6) and the kite cooldown (E12) count DCR *calls*, not rounds, so they can expire within one hit.

**Fix.** Remove the duplicate `SpeakString` at 726 for the leader. Rate-limit DCR from shouts, for example skip when `GetCurrentAction()` is already `ATTACKOBJECT` or `CASTSPELL`. Recompile `k_ai_master`, and the dlg scripts if `GN_RespondToShout` changes.

---

## Side finding (not companions): combat-zone typo in `GN_RespondToShout`

`k_inc_generic.nss:1999`:

```c
1999|    if(GetLocalNumber(oShouter, SW_NUMBER_COMBAT_ZONE) == GetLocalNumber(OBJECT_SELF, SW_NUMBER_LAST_COMBO) ||
```

The right-hand side should be `SW_NUMBER_COMBAT_ZONE` (7), not `SW_NUMBER_LAST_COMBO` (5). Zoned encounter creatures ignore shouts from their own zone. Party listeners short-circuit on `IsObjectPartyMember(OBJECT_SELF)`, and a party shouter has zone 0, which passes. No companion effect; it affects how enemies call for help.

---

## Engine notes relevant to the post-combat-movement-fix question

- The community patch is engine-only. Per the ghidra DB (`hooks_ported`, `post-combat-movement-fix-v1`), it is a 16-byte x87 max-of-two-floats replacement at Steam `0x7AF198`, aimed at high-FPS movement locks. No script change can cause or fix that lock.
- Script-side contributors to "companions don't follow after combat":
  - E1: no exit routine for 4 of the 5 stances;
  - E7: dead exit-5m hook;
  - E8: corpse target;
  - E4: no recovery from a stuck `MOVETOPOINT`, because the heartbeat and the leash both treat any `MOVETOPOINT` as "leave alone". E4 is how the engine lock becomes permanent at script level.

---

## Coverage

**Read fully:**
- `k_ai_master.nss` (all 886 lines: cases 3001-3003, 1001-1012, 2001-2014);
- `k_hen_heartbt01`, `k_hen_percept01`, `k_hen_attacked01`, `k_hen_damage01`, `k_hen_combend01`, `k_hen_dialogue01`, `k_hen_blocked01`, `k_hen_spawn01`, `k_hen_leadchng`, `k_hen_retreat`, `k_hen_enter5m`, `k_hen_exit5m`;
- `k_hen_attondlg`, `baodurdlg`, `discipdlg`, `g0t0dlg` in full; the other 9 `k_hen_*dlg` have identical `main()` bodies (md5) and I skimmed their `SelectAndRunDLG`;
- `a_atkonend`, `a_force_combat`, `k_combat_rnd`;
- `k_oei_spawn`, `k_oei_endconv`, `k_oei_userdef`, `m_spn_visas` (mod142 source), `k_def_pathfail01`;
- `kite_routine.nss`, `build_kite_script.py`;
- in `k_inc_generic`: `GN_DetermineCombatRound` (whole), `GN_DoPostDCRChecks`, `GN_RunDefaultAIRoutine` (targeting and exit part), `GN_RespondToShout`, `GN_SetListeningPatterns`, `GN_SetLastRoundData`, `GN_DetermineAttackTarget`, `GN_CheckRangeFromLeader`, `GN_CheckRangeFromOwner`, `GN_RunRangedSupportAIRoutine`, `GN_CheckRangedPreferred`, flag constants (lines 30-134);
- in `k_inc_gensupport`: `GN_GetIsFighting`, `GN_GetWeaponType*`, `GN_EquipAppropriateWeapon`, constants;
- `k_inc_walkways` constants, `GN_Set/GetSpawnInCondition`, `GN_SetUpWayPoints`;
- nwscript prototypes and docs for every engine call in these handlers.

**Skimmed:**
- `k_oei_hench_inc.nss`: every spawn-in function, the Atton, Remote and 711 user-defined handlers in full, the rest by grep. The 003EBO-only Handmaiden sparring and waypoint logic was skimmed.
- The mod148 diff of `k_oei_hench_inc` (711KOR-only changes).
- EXCERPTS.md §1, §2 and §8; §8 verified against source with my own grep.

**Data checks (read-only):**
- pykotor parse of the live `override/p_*.utc` and BIF `p_atton/p_hk47/p_kreia/p_remote/p_visas`;
- module capsule scan for AI scripts. Only 003EBO has `k_hen_combend01`/`k_hen_damage01`; also 101PER `k_hen_spawn01_hk` (HK-50, not a party member), 305NAR `k_oei_spawn_han`, 711KOR `k_oei_userdef`. No module ships `k_ai_master`.
- exe string scan for engine-called script names;
- md5 of the live `k_ai_master`.

**Not covered:**
- 003EBO `k_hen_combend01`/`k_hen_damage01` (no source);
- the mod `k_def_*` .ncs in `override/`: `ambmobtrea, drdgrenspn, grenspn, repairsptr, spawn01, spn_t_drd/drdh/empt/jedi/less/more/per` (no source; these are for non-party creatures);
- any dialog or module script that runs `a_force_combat`/`k_combat_rnd`/`a_localn_*` on a companion;
- engine behaviour behind these questions:
  - does `GetNearestCreature` skip dead creatures (assumed yes);
  - is `IsObjectPartyMember` FALSE for the Remote puppet;
  - when are attempted and attack targets set;
  - how often do exit-5m and enter-5m fire;
- the decision layer (other reviewer).

---

## Checked, no bug

- **Engine-call argument order.** Every call in the event handlers matches its prototype. Checked: `GetNearestCreature` (2001 uses PERCEPTION-then-REPUTATION, 1001 the reverse; equivalent), `SetCommandable(int, object)`, `GetItemInSlot(slot, obj)`, `GetIsDoorActionPossible/DoDoorAction(door, action)`, `GetAbilityScore(obj, ability)`, `ActionMoveToObject(obj, run, range)`, `ActionMoveToLocation(loc, run)`, `GetIsEnemy`/`GetObjectSeen(target, source)`, `GetLastHostileActor(victim)`, `GetAttackTarget(creature)`, `SpeakString(str, volume)`, `BarkString(obj, strref)`, `HasLineOfSight(v, v, o, o)`, `GetRandomDestination(obj, int)`.
- **No event storms or recursion.** `SignalEvent(EventUserDefined(100x))` goes to `k_oei_userdef`, which never routes back into `k_ai_master`. Jolee, Mission and Zaalbar (K1 leftovers) have an empty OnUserDefined. Shouts only trigger DCR, and DCR does not shout. The Remote's `DelayCommand(2.0, ForceHeartbeat)` chain stops once `ACTION_FOLLOWOWNER` is current. **Caveat:** if `IsObjectPartyMember(Remote)` were TRUE, 2001 (`ActionFollowLeader`) and the user-defined event (`ActionFollowOwner`) would fight every 2 s. The 2001 gate requires `IsObjectPartyMember`, and the puppet faction suggests FALSE; confirm in game if the Remote jitters.
- **Instruction budget (0x20000).** The largest loops here are the zone-controller scan (40), the waypoint count, the kite's 12 tries and the droid LOS search (20). All are far below the cap.
- **DelayCommand / OBJECT_SELF.** Only 1002 (non-party) and `a_atkonend` use DelayCommand. Arguments are evaluated at schedule time, and OBJECT_SELF is the caller. No `GetIsObjectValid` misuse found in the hench cases.
- **End of conversation (1011 via `k_oei_endconv`).** Party members get `CancelCombat` + `ClearAllActions` + `ActionFollowLeader` via `GN_DoPostDCRChecks` when no enemy is seen. Party members are deliberately excluded from the post-conversation DCR (`k_ai_master.nss:454`).
- **`k_hen_retreat`** is a no-op (its whole body is commented out). The old body ended in `SetCommandable(FALSE)` with no matching restore, so leaving it off is correct.
- **Spawn defaults (`k_oei_spawn`).** Listening patterns are set twice (harmless; same indices). No AI stance is forced (the party table and UI own it). Local number 11 is set to 6 (turret cooldown, unused by party AI). `SetIsDestroyable(TRUE,TRUE,TRUE)`. The lowercase tag compares in `DoSpecialSpawnIn` rely on the engine lowercasing tags, which the visibly working 003EBO behaviours confirm. All Ebon Hawk flags are guarded by `GetModuleName()=="003EBO"`.
- **Dialogue handler vs combat.** 2004 and `k_hen_*dlg` start a conversation on `nMatch == -1` with no combat gate (1004 has one). Whether a companion can be talked to in combat is decided by the engine and the module script that starts the conversation. Not a confirmed bug.
- **Perception (2002).** A party member engages a newly seen enemy only when `GetAttackTarget(leader)` is valid. When the leader only casts, the leader's round-end shout brings companions in within one round (see E7). This is design.
- **Leader change.** `k_hen_leadchng:13 SetCommandable(TRUE, GetPartyMemberByIndex(0))` forces the new leader commandable. That can undo a scripted `SetCommandable(FALSE)` on the leader during a cutscene. This is low risk because switching is normally impossible in cutscenes.
- **Solo mode.** 2003, 2005 and 2006 have no solo check. The comment at `k_ai_master.nss:629` claims one was added, but the code has none. Companions left behind still defend themselves, and post-DCR follow is gated by `!GetSoloMode()`. This is acceptable.
- **Local number index 20.** No collision on party members (see E12). The engine doc range "12..28" (`nwscript.nss:5157`) includes 20, and the cooldown values 0-2 are within the 0-255 limit.
